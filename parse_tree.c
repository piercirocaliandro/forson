#include <generation.h>

tree_node *init_tree_node(symbol_id sym){
    tree_node *ret = xmalloc(sizeof(tree_node));
    assert(ret != NULL);

    ret->children = xcalloc(PARSE_TREE_DEFAULT_CHILDREN_NUM, sizeof(tree_node *));
    assert(ret->children != NULL);

    ret->sym = sym;
    ret->children_array_size = PARSE_TREE_DEFAULT_CHILDREN_NUM;
    ret->num_children = 0;
    ret->parent=NULL;
    ret->expanded=0;
    return ret;
}

tree_node *init_empty_tree_node(void){
    tree_node *ret = xmalloc(sizeof(tree_node));
    assert(ret != NULL);

    ret->children = xcalloc(PARSE_TREE_DEFAULT_CHILDREN_NUM, sizeof(tree_node *));
    assert(ret->children != NULL);

    ret->sym = 0;
    ret->children_array_size = PARSE_TREE_DEFAULT_CHILDREN_NUM;
    ret->num_children = 0;
    ret->parent=NULL;
    ret->expanded=0;
    return ret;
}

int tree_node_push_child(tree_node *parent, symbol_id sym){
    // create the child node
    assert(parent != NULL);
    tree_node* child = init_tree_node(sym);
    child->parent=parent;
    if(parent->num_children >= parent->children_array_size){
        // need to resize the array
        parent->children = realloc(parent->children, parent->children_array_size * 2 * sizeof(tree_node *));
        parent->children_array_size *= 2;
    }

    assert(parent->children != NULL);

    // insert the new node in the children array
    parent->children[parent->num_children] = child;
    parent->num_children++;

    // return the actual number of children
    return parent->num_children;

}

/**
 * @brief Must be called only on the root of the parse tree
 * 
 * @param node 
 */

void tree_node_clean(tree_node *node){
    int i;
    assert(node != NULL);
    //assert(node->parent==NULL);
    for(i = 0; i < node->num_children; i++){
        // recursively clean all the subtrees
        tree_node_clean(node->children[i]);
    }
    free(node->children);
    free(node);
}

parse_tree * init_parse_tree(symbol_id sym){
    parse_tree* tree = xmalloc(sizeof(parse_tree));
    assert(tree!=NULL);
    tree->root=init_tree_node(sym);
    return tree;
}

void parse_tree_clean(parse_tree *tree){
    tree_node_clean(tree->root);
    free(tree);
}

int calculate_subtree_depth(tree_node* node) {
    if (node == NULL) {
        return 0;
    }
    
    int max_depth = 0;
    
    for (int i = 0; i < node->num_children; i++) {
        int depth = calculate_subtree_depth(node->children[i]);
        if (depth > max_depth) {
            max_depth = depth;
        }
    }
    
    return max_depth + 1;
}

void stampa_sottoalbero(tree_node* nodo, int livello, int ultimo_ramo[], symbol_list_entry * sym_tab) {
    if (nodo == NULL) {
        return;
    }
    
    // Stampa l'indentazione in base al livello del nodo
    for (int i = 0; i < livello; i++) {
        if (ultimo_ramo[i] == 0) {
            printf("│  ");
        } else {
            printf("   ");
        }
    }
    
    if (livello > 0) {
        printf("├─");
    }
    
    printf(" %s\n", get_symbol(sym_tab,nodo->sym)->name);  // Stampa il valore sym del nodo
    
    int num_figli = nodo->num_children;
    
    for (int i = 0; i < num_figli; i++) {
        ultimo_ramo[livello] = (i == num_figli - 1) ? 1 : 0;
        stampa_sottoalbero(nodo->children[i], livello + 1, ultimo_ramo,sym_tab);
    }
}


void print_tree(tree_node *root, symbol_list_entry* sym_tab){
    size_t depth = calculate_subtree_depth(root);
    int ultimo_ramo[depth];
    int i;
    for(i=0;i<depth;i++){
        ultimo_ramo[i]=0;
    }
    stampa_sottoalbero(root,0,ultimo_ramo,sym_tab);
}