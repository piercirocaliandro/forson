#include <generation.h>

parse_tree *init_parse_tree(symbol_id sym){
    parse_tree *ret = xmalloc(sizeof(parse_tree));
    assert(ret != NULL);

    ret->children = xcalloc(PARSE_TREE_DEFAULT_CHILDREN_NUM, sizeof(parse_tree *));
    assert(ret->children != NULL);

    ret->sym = sym;
    ret->children_array_size = PARSE_TREE_DEFAULT_CHILDREN_NUM;
    ret->num_children = 0;

    return ret;
}

parse_tree *init_empty_parse_tree(void){
    parse_tree *ret = xmalloc(sizeof(parse_tree));
    assert(ret != NULL);

    ret->children = xcalloc(PARSE_TREE_DEFAULT_CHILDREN_NUM, sizeof(parse_tree *));
    assert(ret->children != NULL);

    ret->sym = 0;
    ret->children_array_size = PARSE_TREE_DEFAULT_CHILDREN_NUM;
    ret->num_children = 0;

    return ret;
}

int parse_tree_push_child(parse_tree *tree, symbol_id sym){
    // create the child node
    assert(tree != NULL);
    parse_tree* child = init_parse_tree(sym);

    if(tree->num_children >= tree->children_array_size){
        // need to resize the array
        tree->children = realloc(tree->children, tree->children_array_size * 2 * sizeof(parse_tree *));
        tree->children_array_size *= 2;
    }

    assert(tree->children != NULL);

    // insert the new node in the children array
    tree->children[tree->num_children] = child;
    tree->num_children++;

    // return the actual number of children
    return tree->num_children;

}

void parse_tree_clean(parse_tree *tree){
    int i;
    assert(tree != NULL);
    for(i = 0; i < tree->num_children; i++){
        // recursively clean all the subtrees
        parse_tree_clean(tree->children[i]);
    }
    free(tree->children);
    free(tree);
}