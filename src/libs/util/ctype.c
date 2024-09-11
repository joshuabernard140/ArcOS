#include "ctype.h"

bool islower(char character) {
    return character >= 'a' && character <= 'z';
}

char toupper(char character) {
    return islower(character) ? (character - 'a' + 'A') : character;
}