(DEFINE (G X)
	   (IF (= X 1) 1
        (+ (/ 1 X) (G (+ X -1)))))
(DEFINE (H X)
	   (IF (= X 1) 1
        (* (/ 1 X) (H (+ X -1)))))

(DEFINE (D X)
	   (IF (= X 1) 1
        (/ (/ 1 X) (D (+ X -1)))))

(DEFINE (TESTNUM N)
	   (IF (= N 0) NIL
		   (CONS (G N) 
		    (CONS (H N) 
			(CONS (D N)
			 (CONS (TESTNUM (+ N -1))))))))

(DEFINE A (TESTNUM 10))
(DEFINE B (TESTNUM 20))
(DEFINE C (TESTNUM 30))

(DEFINE (GC X)
	   (IF (= X 0) NIL
        (list (LIST NIL NIL t nil t nil t nil a a a a b c c c c) (GC (+ X -1)))))

(DEFINE (HC X)
	   (IF (= X 0) NIL
        (LIST (LIST 'A A 'B B 'C C t t t nil nil t) (HC (+ X -1)))))

(DEFINE (DC X)
	   (IF (= X 0) NIL
        (CONS (LIST A B C a b c a b c) (CONS (DC (+ X -1))))))
        

(DEFINE (TESTCONS N)
	   (IF (= N 0) NIL
		   (CONS (GC N) 
		    (CONS (HC N) 
			(CONS (DC N)
			 (CONS (TESTCONS (+ N -1))))))))

(DEFINE (TEST N)
    (IF (= N 0) NIL
	(CONS (TESTNUM N) (CONS (TESTCONS N) (TEST (+ N -1))))))


(DEFINE (STRESS )
(PROGN
(TEST 38) (TEST 39)
))
(TEST 20) 
(TEST 36) 

(STRESS)
(STRESS)
(STRESS)
(STRESS)
