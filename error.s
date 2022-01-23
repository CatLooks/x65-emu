@ ROM header
header:
	.byte "x65", $00
	.word $01

@ program
PRG:
	.spos $F000
	.fpos $0010
	.set ERR $00

@ empty interrupt
int:
	rti

@ output message
print:
	ltd y
	beq :+
	std $4000
	iny
	bra print
:
	rts

@ entry code
rst:
	@ initialize CPU
	sei
	clf
	ltx #$1000
	ltv x
	ltx #$1FFF
	txs

	@ create palette
	ltx #$00
	stx $4004
:
	lta palette, x
	sta $4000
	inx
	inx
	cpx #$200
	bcc :-

	@ reset background
	ltx #$8000
	stx $4004
	lta #$00
	tax
:
	sta $4000
	inx
	inx
	cpx #$4B0
	bcc :-

	ltx #$C000
	stx $4004
	lta #$00
	tax
:
	sta $4000
	inx
	inx
	cpx #$4B0
	bcc :-

	@ copy error message
	ltx #$8029
	stx $4004
	lty #error00
	jsr print

	ltx #$8079
	stx $4004
	lta #$00
	ltd ERR
	asl
	tax
	lty errorRefs, x
	jsr print

	@ enable GPU
	ltd #%00100000
	std $4002
	stz $4003

	@ infinite loop
infloop:
	wai
	bra infloop

@ error messages
error00:
	.byte "FAILED TO LOAD FILE:", $00
error10:
	.byte "@ TOO SMALL FILE", $00
error11:
	.byte "@ TOO LARGE PROGRAM ROM", $00
error12:
	.byte "@ TOO LARGE AUDIO ROM", $00
error13:
	.byte "@ TOO SMALL PROGRAM ROM", $00
error14:
	.byte "@ CHARACTER ROM MUST BE ALIGNED", $00
error15:
	.byte "@ TOO LARGE CHARACTER ROM", $00
error16:
	.byte "@ INVALID FILE SIGNATURE", $00
errorRefs:
	.fill $14
	.word error10
	.word error11
	.word error12
	.word error13
	.word error14
	.word error15
	.word error16

@ palette
palette:
	.byte $11 $10, $11 $10, $FF $FF, $DD $DF

@ vectors
vectors:
	.fpos $100A
	.word int
	.word rst
	.word int

@ graphics
CHR:
	.fpos $1010
	.file "graphics.chr"