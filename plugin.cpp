#include <gcc-plugin.h>
#include <cp/cp-tree.h>
#include <c-family/c-common.h>
#include <tree.h>
#include <tree-nested.h>
#include <print-tree.h>
#include <tree-iterator.h>
#include <plugin-version.h>
#include <stringpool.h>
#include <attribs.h>

#include <stdlib.h>
#include <stdio.h>

// Licensed under 0-Clause BSD
int plugin_is_GPL_compatible;

#define PLUGIN_NAME "gcc-force-constant-evaluated"
#define ATTRIBUTE_NAME "force_constant_evaluated"

static struct plugin_info my_plugin_info =
{
    .version = "0.1",
    .help = "Add a function attribute `" ATTRIBUTE_NAME "` which treats all calls to a function as constant expressions."
};

static tree handle_force_consteval_attribute(
    tree * node, tree name, tree /*args*/, int /*flags*/, bool * no_add_attrs)
{
    if (TREE_CODE (*node) == FUNCTION_DECL)
    {
        if (!DECL_DECLARED_CONSTEXPR_P(*node))
        {
            warning (OPT_Wattributes, "%qE attribute on a function not declared constexpr", name);
        }
    }
    else
    {
      warning (OPT_Wattributes, "%qE attribute ignored: Not on a function declaration", name);
      *no_add_attrs = true;
    }

    return NULL_TREE;
}

static attribute_spec force_consteval_attr {
    .name = ATTRIBUTE_NAME,
    .min_length = 0,
    .max_length = 0,
    .decl_required = true,
    .type_required = false,
    .function_type_required = false,
    .affects_type_identity = false,
    .handler = handle_force_consteval_attribute,
    .exclude = nullptr
};

// Callback called during attribute registration
static void callback_register_attributes (void * /*event_data*/, void * /*data*/)
{
    register_attribute (&force_consteval_attr);
}

static bool is_call_to_force_consteval_fn(tree t) {
    if (tree fnDecl = cp_get_callee_fndecl(t))
    {
        tree declAttrs = DECL_ATTRIBUTES(fnDecl);
        if (lookup_attribute(ATTRIBUTE_NAME, declAttrs))
        {
            return true;
        }

        if (tree declType = TREE_TYPE(fnDecl))
        {
            tree typeAttrs = TYPE_ATTRIBUTES(declType);
            if (lookup_attribute(ATTRIBUTE_NAME, typeAttrs))
            {
                return true;
            }
        }
    }

    return false;
}

void handle_pre_generic (void *event_data, void * /*data*/)
{
    // The current function being parsed
    tree fnDecl = (tree) event_data;
    tree fnBody = DECL_SAVED_TREE (fnDecl);

    auto walkFunc = [](tree * t, int *, void *) -> tree
    {
        if (t != nullptr && is_call_to_force_consteval_fn(*t))
        {
            *t = cxx_constant_value(*t);
        }

        return NULL_TREE;
    };

    walk_tree_without_duplicates(&fnBody, walkFunc, nullptr);
}

// Returns 0 on success, nonzero on failure
int plugin_init(struct plugin_name_args * /*plugin_info*/, plugin_gcc_version *version)
{
    if (!plugin_default_version_check(version, &gcc_version))
    {
        fprintf(stderr, "Error: Wrong gcc version for plugin " PLUGIN_NAME "\n");
        return 1;
    }

    // Register plugin info
    register_callback(PLUGIN_NAME, PLUGIN_INFO, NULL, &my_plugin_info);

    // Register callback to add custom attributes
    register_callback (PLUGIN_NAME, PLUGIN_ATTRIBUTES, callback_register_attributes, NULL);

    // Register callback to run in the pre-generic phase
    register_callback (PLUGIN_NAME, PLUGIN_PRE_GENERICIZE, handle_pre_generic, NULL);
    
    return 0;
}
