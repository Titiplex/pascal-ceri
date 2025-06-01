VAR
    i, s : INTEGER.
BEGIN
    i := 1;
    s := 0;
    WHILE i <= 5 DO
    BEGIN
        s := s + i;
        i := i + 1
    END;
    DISPLAY s
END.
