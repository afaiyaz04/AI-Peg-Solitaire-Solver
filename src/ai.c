#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
	node_t* n = solution_node;
	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;

		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action ){

    node_t* new_node = NULL;
	new_node = (node_t *) malloc(sizeof(node_t));
	assert(new_node!=NULL);

	// Setting the parent and applying the actions in the new state
	// Updating the depth of the new node
	new_node->parent = n;
	copy_state(&(new_node->state), &(n->state));
	execute_move_t( &(new_node->state), selected_peg, action );
	new_node->depth = (n->depth) + 1;
	new_node->move = action;
	
	return new_node;

}
/**
 * Find a solution path as per algorithm description in the handout
 * Function edited by Ahbab Faiyaz 1045377
 * On 1st November 2020
 */

void find_solution( state_t* init_state  ){

	HashTable table;

	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup( &table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);

	// Initialize Stack
	initialize_stack();

	// Setting up variable to store remainig pegs
	int remainingPeg;

	// Initialize a list to keep track of the nodes visited
	list_t* visited =make_empty_list();

	// Create the initial node
	// Count the number of pegs in it 
	node_t* n = create_init_node( init_state );
	insert_at_head(visited, n);
	stack_push(n);
	remainingPeg = num_pegs(&(n->state));
	// We will keep checking until we find new states
	// Increment the expanded node counter
	while(!is_stack_empty()){
		n = stack_top();
		stack_pop();
		ht_insert(&table, (n->state.field), (n->state.field));
		expanded_nodes++;
		// A state will lesser peg is considered a better solution
		// It is saved as the best solution
		if(num_pegs(&(n->state)) < remainingPeg){
			save_solution(n);
			remainingPeg = num_pegs(&(n->state));
		}

		// Will chack through the whole board to see if a legal move can be made
		for(int8_t x=0; x < SIZE ; x++){
			for(int8_t y=0; y < SIZE; y++){

				position_s position;
				position.x = x;
				position.y = y;
				// Four direction is checked for each peg
				for(int pegDir=0; pegDir < DIRECTION ; pegDir++){
					// When a legal move can be made
					// A new node with the applied state is made
					if(can_apply(&(n->state), &position, pegDir)){
						node_t *new_node = applyAction(n, &position, pegDir);
						insert_at_head(visited, new_node);
						generated_nodes++;
						// If the new node is the final solution
						// solution is saved and all the allocated memory is freed
						if(won(&(new_node->state))){
							save_solution(new_node);
							remainingPeg = num_pegs(&(new_node->state));
							free_list(visited);
							ht_destroy(&table);
							return;
						}
						// When a new state is found 
						// It is pushed in the stack to be explored further
						if(!ht_contains(&table, new_node->state.field)){
							stack_push(new_node);
						}
					}
				}
			}
		}
		// The search continues until max budget is reached
		if(expanded_nodes >= budget){
			free_list(visited);
			ht_destroy(&table);
			return;
		}

	}
	
}