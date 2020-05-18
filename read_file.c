#include <stdio.h>
#include <stdlib.h> // For exit() function
#include <string.h>


typedef struct pos_board{
  char object;
  int sock_id;
  int r;
  int g;
  int b;
}pos_board;



int main() {


  int col;
  int line;

  FILE *fp = fopen("board.txt", "r");
  // test for files not existing. 
  if (fp == NULL) 
  {   
    printf("Error! Could not open file\n"); 
    exit(-1); 
  }

    //Reads first line of 'fp' file
   fscanf(fp, "%d %d", &col, &line);

   printf("Cols: %d\n", col);
   printf("Lines: %d\n", line);


   char letter;

   int i;
   int j;


  pos_board board[line][col];
  for ( i = 0 ; i < line; i++){
    for (j = 0; j < col; j++){

     board[i][j].object = ' ';

    }
  }

  j=0;
  i=-1;

/*  int i, j;
  board = malloc(sizeof(pos_board) * (line));           
  for ( i = 0 ; i < line; i++){
    board[i] = malloc (sizeof(pos_board) * (col));
    for (j = 0; j < col; j++){
      board[i][j].object = ' ';
    }
    
  }*/



  while( (letter = getc(fp)) != EOF){    
    //Block
    if(letter == 'B') {
      board[i][j].object = 'B';
    }
    //New Column
    j++;
    //New Line
    if (letter == '\n'){
      i++;
      j = 0;
      } 
  }
  fclose(fp);

int aux = 0;
int aux2 = 0;

  for ( i = 0 ; i < line; i++){
    for (j = 0; j < col; j++){

      printf("%c", board[i][j].object);
      aux2++;

    }
    printf("\n");
    aux++;
  }

printf("-------------%d\n", aux);
printf("-------------%d\n", aux2);

  return 0;
}