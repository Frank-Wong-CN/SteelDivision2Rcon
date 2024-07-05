#ifndef MACRO_H
#define MACRO_H

/**
 * Line 11-99: Macro implementation of creating nested initializer-lists
 *   Source: https://stackoverflow.com/a/62490101/10423689
 *           http://coliru.stacked-crooked.com/a/1bfca2c6d3278722
 *   Author: H Walters @ StackOverflow
 */

// Indirect glue
#define GLUE(A,B) GLUE_I(A,B)
#define GLUE_I(A,B) A##B

// Convert a tuple to a sequence
#define PARN(...) GLUE(PARN_, COUNT (__VA_ARGS__)) (__VA_ARGS__)
#define PARN_9(A,B,C,D,E,F,G,H,I) (A)(B)(C)(D)(E)(F)(G)(H)(I)
#define PARN_8(A,B,C,D,E,F,G,H) (A)(B)(C)(D)(E)(F)(G)(H)
#define PARN_7(A,B,C,D,E,F,G) (A)(B)(C)(D)(E)(F)(G)
#define PARN_6(A,B,C,D,E,F) (A)(B)(C)(D)(E)(F)
#define PARN_5(A,B,C,D,E) (A)(B)(C)(D)(E)
#define PARN_4(A,B,C,D) (A)(B)(C)(D)
#define PARN_3(A,B,C) (A)(B)(C)
#define PARN_2(A,B) (A)(B)
#define PARN_1(A) (A)

// Appends _E to rhs; for sequence iteration
#define PASTE_E(...) PASTE_E_I(__VA_ARGS__)
#define PASTE_E_I(...) __VA_ARGS__ ## E

// Counter... up to 9 shown for example
#define COUNT(...) COUNT_I(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,)
#define COUNT_I(_,_10,_9,_8,_7,_6,_5,_4,_3,_2,X,...) X

// Indirect second used as pattern matcher
#define SECOND(...) SECOND_I(__VA_ARGS__,,)
#define SECOND_I(A,B,...) B

// Used with SECOND to detect if X is parenthetical;
// e.g., SECOND( CALLDETECT X WHAT_I_WANT )
// ...expands to WHATIWANT if X is parenthetical, nothing otherwise
#define CALLDETECT(...) ,

// A simple terminating bracify
#define BRACIFY(...) {__VA_ARGS__}

// Top level magic
#define MAGIC(...) {PASTE_E(MAGIC_U(BRAC0_A PARN(__VA_ARGS__)))}
// Each magic calls its own unwrap
#define MAGIC_U(...) __VA_ARGS__
// bracify-parenthetical, or leave non-parenthetical alone.
// For level 0.  (Apparently level 0 only is surrounded by extra {}'s?)
#define BRACP0(X) {SECOND(CALLDETECT X MAGIC1)X}
// Sequencer to generate comma-delimited calls to BRACP0
#define BRAC0_A(X) BRACP0(X)BRAC0_B
#define BRAC0_B(X) ,BRACP0(X)BRAC0_C
#define BRAC0_C(X) ,BRACP0(X)BRAC0_B
#define BRAC0_BE
#define BRAC0_CE

// Level 1 magic, bracp, sequencers
#define MAGIC1(...) {PASTE_E(MAGIC1_U(BRAC1_A PARN(__VA_ARGS__)))}
#define MAGIC1_U(...) __VA_ARGS__
#define BRACP1(X) SECOND(CALLDETECT X MAGIC2)X
#define BRAC1_A(X) BRACP1(X)BRAC1_B
#define BRAC1_B(X) ,BRACP1(X)BRAC1_C
#define BRAC1_C(X) ,BRACP1(X)BRAC1_B
#define BRAC1_BE
#define BRAC1_CE

// Level 2 magic, bracp, sequencers
#define MAGIC2(...) {PASTE_E(MAGIC2_U(BRAC2_A PARN(__VA_ARGS__)))}
#define MAGIC2_U(...) __VA_ARGS__
#define BRACP2(X) SECOND(CALLDETECT X MAGIC3)X
#define BRAC2_A(X) BRACP2(X)BRAC2_B
#define BRAC2_B(X) ,BRACP2(X)BRAC2_C
#define BRAC2_C(X) ,BRACP2(X)BRAC2_B
#define BRAC2_BE
#define BRAC2_CE

// Level 3 magic, bracp, sequencers
#define MAGIC3(...) {PASTE_E(MAGIC3_U(BRAC4_A PARN(__VA_ARGS__)))}
#define MAGIC3_U(...) __VA_ARGS__
#define BRACP3(X) SECOND(CALLDETECT X MAGIC4)X
#define BRAC3_A(X) BRACP3(X)BRAC3_B
#define BRAC3_B(X) ,BRACP3(X)BRAC3_C
#define BRAC3_C(X) ,BRACP3(X)BRAC3_B
#define BRAC3_BE
#define BRAC3_CE

// Level 4 magic, bracp, sequencers
#define MAGIC4(...) {PASTE_E(MAGIC4_U(BRAC3_A PARN(__VA_ARGS__)))}
#define MAGIC4_U(...) __VA_ARGS__
#define BRACP4(X) SECOND(CALLDETECT X BRACIFY)X
#define BRAC4_A(X) BRACP4(X)BRAC4_B
#define BRAC4_B(X) ,BRACP4(X)BRAC4_C
#define BRAC4_C(X) ,BRACP4(X)BRAC4_B
#define BRAC4_BE
#define BRAC4_CE

#define NEST(...) GLUE(NEST_, COUNT (__VA_ARGS__)) (__VA_ARGS__)
#define NEST_4(A,B,C,D) (A,(B,(C,D)))
#define NEST_3(A,B,C) (A,(B,C))
#define NEST_2(A,B) (A,B)
#define NEST_1(A) (A)

#define FIRST(...) FIRST_I(__VA_ARGS__,)
#define FIRST_I(A,...) A

#ifndef ARG_ITER_DO
#define ARG_ITER_DO(X)
#endif

#define ARG_ITER_UNWRAP(...) __VA_ARGS__
#define ARG_ITER_A(X) ARG_ITER_DO(X) ARG_ITER_B
#define ARG_ITER_B(X) ARG_ITER_DO(X) ARG_ITER_C
#define ARG_ITER_C(X) ARG_ITER_DO(X) ARG_ITER_B
#define ARG_ITER_BE
#define ARG_ITER_CE
#define ARG_ITER_I(...) PASTE_E(ARG_ITER_UNWRAP(ARG_ITER_A PARN(__VA_ARGS__)))
#define ARG_ITER(...) __VA_OPT__(ARG_ITER_I(__VA_ARGS__))

#define NOT_DEFAULT_CONSTRUCTIBLE(X) X() = delete;
#define NOT_COPIABLE(X) X(const X&) = delete;
#define NOT_MOVABLE(X) X(X&&) noexcept = delete;
#define NOT_COPY_ASSIGNABLE(X) X &operator=(const X&) = delete;
#define NOT_MOVE_ASSIGNABLE(X) X &operator=(X&&) noexcept = delete;

#endif // MACRO_H
