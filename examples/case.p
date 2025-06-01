VAR
    note : INTEGER.
BEGIN
    note := 18;
    CASE note OF
        0,1,2,3,4 : DISPLAY "F";
        5,6,7,8,9 : DISPLAY "E";
        10,11,12,13 : DISPLAY "D";
        14,15,16 : DISPLAY "C";
        17,18 : DISPLAY "B";
        19,20 : DISPLAY "A"
    END
END.
