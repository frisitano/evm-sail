; Sanity check for the pure schema-prefix relation lowered from evm-sail.
;
; evm-sail parses the two-byte schema id as big-endian:
;   schema_id = byte0 * 256 + byte1
; Therefore schema id 1 is prefix 00 01.

(set-logic QF_BV)

(declare-fun len () (_ BitVec 64))
(declare-fun byte0 () (_ BitVec 8))
(declare-fun byte1 () (_ BitVec 8))

(define-fun len_ge_2 () Bool (bvuge len (_ bv2 64)))

(define-fun spec_ok () Bool
  (and len_ge_2
       (= byte0 #x00)
       (= byte1 #x01)))

; Shape expected from a RISC-V slice that checks byte0 == 0 and byte1 == 1.
(define-fun riscv_ok () Bool
  (ite (not len_ge_2)
       false
       (= (bvor byte0 (bvxor byte1 #x01)) #x00)))

(assert (xor spec_ok riscv_ok))
(check-sat)
