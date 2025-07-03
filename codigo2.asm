GLOBAL _print_val      // indica que _print_val é GLOBAL
EXTERN _val_to_print   // indica que vai usar EXTERN _val_to_print

_print_val:
    w _val_to_print    // printa o valor armazenado
    stp                // stop
