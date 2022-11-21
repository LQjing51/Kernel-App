#include "hook.h"
#include "common.h"

#define HOOK(_name, _function, _original) {     \
                .name = (_name),                \
                .function = (_function),        \
                .original = (_original),        \
        }

struct ftrace_hook {
    const char *name;
    void *function;
    void *original;
    unsigned long address;
    struct ftrace_ops ops;
};

#define USE_FENTRY_OFFSET 0

long (*ORIG(execve))(const struct pt_regs *);


int is_monitor_app(struct mm_struct *mm) {
    char *pathbuf, *path, *suffix;
    struct file *exe_file;
    int ret;

    exe_file = ORIG(get_mm_exe_file)(mm);   
    if (!exe_file)
        return 0;

    pathbuf = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!pathbuf) 
        return -ENOMEM;
    
    path = file_path(exe_file, pathbuf, PATH_MAX);
    if (IS_ERR(path)) {
		ret = PTR_ERR(path);
		goto free_buf;
	}

    suffix = strrchr(path, '.');
    if (!suffix) {
        ret = 0;
        goto free_buf;
    }

    ret = (strncmp(suffix, ".spec", 5) == 0);
    
free_buf:
    kfree(pathbuf); 
    return ret; 
}


long __intercept_execve(const struct pt_regs *regs) {
    long ret; 

    ret = orig_execve(regs);	

    if (is_monitor_app(current->mm) != 1) { return ret; }

    INFO("Find target app!\n");
    return ret;
}


static int fh_resolve_hook_address(struct ftrace_hook *hook) {
   	hook->address = kln(hook->name);
    if (!hook->address) {
        ERR("unresolved symbol: %s\n", hook->name);
        return -ENOENT;
    }

#if USE_FENTRY_OFFSET
    *((unsigned long*) hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
    *((unsigned long*) hook->original) = hook->address;
#endif

    return 0;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
        struct ftrace_ops *ops, struct ftrace_regs *regs) {
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
    regs->regs.pc = (unsigned long) hook->function;
#else
    if (!within_module(parent_ip, THIS_MODULE))
        regs->regs.pc = (unsigned long) hook->function;
#endif
}

/**
 * fh_install_hooks() - register and enable a single hook
 * @hook: a hook to install
 *
 * Returns: zero on success, negative error code otherwise.
 */
int fh_install_hook(struct ftrace_hook *hook) {
    int err, err1;

    err = fh_resolve_hook_address(hook);
    if (err)
        return err;

    /*
     * We're going to modify %rip register so we'll need IPMODIFY flag
     * and SAVE_REGS as its prerequisite. ftrace's anti-recursion guard
     * is useless if we change %rip so disable it with RECURSION_SAFE.
     * We'll perform our own checks for trace function reentry.
     */
    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
        // | FTRACE_OPS_FL_RECURSION_SAFE
        | FTRACE_OPS_FL_IPMODIFY;

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if (err) {
        err1 = ftrace_set_filter(&hook->ops, (unsigned char *)hook->name, 0, 0);
        if (err1) {
            ERR("both ftrace_set_filter_ip() and ftrace_set_filter failed: %d, %d\n", err, err1);
            return err1;
        }
    }

    err = register_ftrace_function(&hook->ops);
    if (err) {
        ERR("register_ftrace_function() failed: %d\n", err);
        err1 = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
        if (err1)
            ftrace_set_filter(&hook->ops, (unsigned char *)hook->name, 1, 0);
        return err;
    }

    return 0;
}

/**
 * fh_remove_hooks() - disable and unregister a single hook
 * @hook: a hook to remove
 */
void fh_remove_hook(struct ftrace_hook *hook) {
    int err, err1;

    err = unregister_ftrace_function(&hook->ops);
    if (err) {
        ERR("unregister_ftrace_function() failed: %d\n", err);
    }

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if (err) {
        err1 = ftrace_set_filter(&hook->ops, (unsigned char *)hook->name, 1, 0);
        if (err1) {
            ERR("both ftrace_set_filter_ip() and ftrace_set_filter failed: %d, %d\n", err, err1);
        }
    }
}

/**
 * fh_install_hooks() - register and enable multiple hooks
 * @hooks: array of hooks to install
 * @count: number of hooks to install
 *
 * If some hooks fail to install then all hooks will be removed.
 *
 * Returns: zero on success, negative error code otherwise.
 */
int fh_install_hooks(struct ftrace_hook *hooks, size_t count) {
    int err;
    size_t i;

    for (i = 0; i < count; i++) {
        err = fh_install_hook(&hooks[i]);
        if (err)
            goto error;
    }

    return 0;

error:
    while (i != 0) {
        fh_remove_hook(&hooks[--i]);
    }

    return err;
}

/**
 * fh_remove_hooks() - disable and unregister multiple hooks
 * @hooks: array of hooks to remove
 * @count: number of hooks to remove
 */
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count) {
    size_t i;

    for (i = 0; i < count; i++)
        fh_remove_hook(&hooks[i]);
}

static struct ftrace_hook hooks[] = {
    /* Functions hooked */
    HOOK("__arm64_sys_execve", __intercept_execve, &orig_execve),
};

int install_hooks(void) {
    int err;

    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if (err)
        return err;

    INFO("install hooks\n");
    return 0;
}

void remove_hooks(void) {

    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));

    INFO("uninstall hooks\n");
}
