/* Optimized C carries an operand-stack cursor as its native word pointer and
 * semantic height. The external representation is declared by
 * evmsail/host/stack.h; canonical and proof backends retain the Sail record. */

$[c_repr { representation = external, name = StackPointer }]
struct StackPointer = {
    storage : bits(64),
    height : operand_stack_height,
}
