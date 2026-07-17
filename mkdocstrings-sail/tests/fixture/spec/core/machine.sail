/*! The program counter. */
register PC : word

val step : unit -> unit

/*! Advance the machine by incrementing `PC`. */
function step() = {
    PC = increment(PC)
}

val flag_of : bool -> bits(1)

/*! Encode a flag via the `flag_bit` mapping. */
function flag_of(b) = flag_bit(b)
