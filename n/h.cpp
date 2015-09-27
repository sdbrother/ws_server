#include <ncurses.h>


int main()
{
    initscr();                 /* Start curses mode     */

    for (int i=0;i<100;i++){
        printw("Hello World !!!"); /* Print Hello World    */
        refresh();                 /* Print it on to the real screen */
        getch();                   /* Wait for user input */
        sleep(1);
    }
    //endwin();                  /* End curses mode    */
    return 0;
}
