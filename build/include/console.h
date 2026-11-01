// console.h - Console série : l'UART2 passe par le câble USB jusqu'au PC (115200 bauds)
// Après console_init(), printf() affiche le texte dans le moniteur série.
#ifndef CONSOLE_H
#define CONSOLE_H

void console_init(void);
int  console_lire_caractere(char *c);   // 1 si une touche a été tapée (la lettre est rangée dans c)

#endif
