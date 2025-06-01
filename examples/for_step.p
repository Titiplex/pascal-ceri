VAR
    acc,i : INTEGER.
BEGIN
    acc := 0;
    FOR i := 0 TO 10 STEP 2 DO
        acc := acc + i;
    DISPLAY acc
END.
