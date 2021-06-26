Data SEGMENT
	string DB "HELLO WORLD! $"
data ENDS

CODE SEGMENT

	      ASSUME CS:code,DS:data
	start:
	      MOV    AX,data
	      MOV    DS,AX
	      MOV    DX,OFFSET string
	      MOV    AH,9
	      INT    21H
	      MOV    AH,4CH
	      INT    21H
code ENDS

END start