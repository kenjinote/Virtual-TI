_DisableInts:
        move.l #$300,d0
        trap #1
        rts

_EnableInts:
        move.l #0,d0
        trap #1
        rts
