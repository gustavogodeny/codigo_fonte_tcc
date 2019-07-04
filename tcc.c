#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>


#define SIZE 50
#define SAIDA 1024
#define HISTORICO 10

//***************************************************************Restrições graves
//R1	Aulas: todas as aulas de um curso devem ser agendadas, e elas devem ser associadas a diferentes 
	//períodos. Uma violação ocorre se uma aula não é agendada;

//R2	Conflitos: Aulas de Disciplinas de um mesmo currículo ou ensinadas pelo mesmo professor devem ser
	// agendadas todas em períodos diferentes. Duas aulas conflitantes no mesmo período representam uma
	// violação. Três aulas conflitantes contam como três violações: uma para cada par;

//R3	Ocupação das salas: Duas aulas não podem ser associadas à mesma sala no mesmo período. Duas aulas na
	// mesma sala e no mesmo período representam uma violação. Qualquer aula extra na mesma sala e no 
	//mesmo período conta como uma violação extra;

//R4	Disponibilidade: Se o professor da disciplina não está disponível para ministrar esta disciplina em
	// um dado período, então nenhuma aula dessa disciplina pode ser agendada para tal período. Cada aula
	// em um período indisponível para tal disciplina é uma violação.
//***************************************************************

//***************************************************************Restrições leves
//R5	Quantidade mínima de dias de trabalho: As aulas de cada disciplina devem ser distribuída de acordo
	// com o número mínimo de dias dado. Cada dia abaixo do mínimo conta como 5 pontos de penalidade;

//R6	Compacidade do currículo: Aulas pertencentes a um currículo deveriam ser adjacentes entre si, isto é,
	// em períodos consecutivos. Para um dado currículo conta-se como uma violação toda vez  que há uma
	// aula não adjacente a qualquer outra aula no mesmo dia. Cada aula isolada em um currículo conta
	// como 2 pontos de penalidade;

//R7	Capacidade da sala: Para cada aula, o número de estudantes inscritos na discpiplina deve ser menor
	// ou igual ao número de assentos de todas as salas que são usadas para suas aulas. Cada estudante
	// além da capacidade conta como 1 ponto de penalidade;

//R8	Estabilidade de salas: Todas as aulas de uma disciplina deveriam ser ministradas em uma mesma sala.
	//Para cada sala distinta usada para as aulas de uma disciplina, sem contar a primeria, conta como 
	//1 ponto de penalidade.  

//Representação de salas no eixo X para dias/períodos no eixo Y. O campo representa a disciplina
typedef struct matriz{
	int fo;
	int** n;
}Matriz;

typedef struct professores{
	char nome[SIZE];
}Professores;

typedef struct disciplina{
	char nome[SIZE];
	int* cursos;	//Campo extra para informar a qual/quais cursos a disciplina pertence
	int  prof;
	char profe[SIZE];
	int  aulas;
	int  minDias;
	int  alunos;
}Disciplina;

typedef struct sala{
	char nome[SIZE];
	int capacidade;
}Sala;

typedef struct curso{
	char nome[SIZE];
	int  qtDisc;
	int* disciplina;
}Curso;

typedef struct restricao{
	int disciplina;
	int dia;
	int per;
}Restricao;

int execucao;
int rotina = 4;
int programa;
int mat_solucao_tempo[HISTORICO][2];
int aux_mat = 0;
int num_exec = 12;

//Variáveis que armazenam e tratam a entrada do arquivo
Professores *prof;
 Disciplina *disc;
       Sala *sala;
      Curso *curso;
  Restricao *restricao;

//Variáveis de controle
int* r1;//[disciplinas];			//R1	Aulas
int** r21;//[total_periodos][professores];	//R2	Conflitos professor
int** r22;//[total_periodos][cursos];		//R2	Conflitos curso
int** r5;//[disciplinas][dias];			//R5	Quantidade mínima de dias de trabalho/Cada dia
int* r8;//[disciplinas];			//R8	Estabilidade de salas

int restricoes_violadas[9];	//Cada posição a partir da segunda representa uma das restrições
int* posicao_restricao[2];	//Início e fim

int* aux_mov_r21;
int* aux_mov_r22;
int* aux_mov_r4;
int* aux_mov_r5;
int** aux_mov_r6;//[total_periodos][salas];		//R6	Disciplina isolada no período/Cada sala
int** aux_mov_r7;
int* aux_mov_r8;

char  nome[SIZE];
int  professores;
int  disciplinas;
int        salas;
int         dias;
int periodos_dia;
int total_periodos;
int       cursos;
int   restricoes;

float Tinicial, T, Tfinal, alpha;
int maxIteracoes;

int modulo(int n1, int n2){
	if(n1 > n2) return n1 - n2;
	else return n2 - n1;
}

int numDisciplina(char *nome){
	int i;
	for(i = 0; i < disciplinas; i++) if(strcmp(nome, disc[i].nome) == 0) return i;
	return -1;
}

int numProf(char *nome){
	int i;
	for(i = 0; i < disciplinas; i++) if(strcmp(nome, prof[i].nome) == 0) return i;
	return -1;
}

double randomDouble(double inicio, double fim){
	return ((double) (rand() % 10000) / 10000.0) * (fim-inicio) + inicio;
}
int randomInt(int inicio, int fim){
	return (int) randomDouble(0, fim - inicio + 1.0) + inicio;
}


int leArquivos(char *arquivo){
	strcpy(nome, "");	//Inicializa variável de nome
	FILE *fp;		//Arquivo de entrada
	int i, c, aux;		//Variáveis contadoras
	int tipo = 0;		//Variável que faz o controle de tipo de inserção do arquivo
	char x[SIZE * SIZE];	//Variável temporária de leitura
	char *lc;
	char char_aux[SIZE];

	strcpy(x, ""); //Inicialização		
			
	//Faz a abertura do arquivo de entrada
	fp = fopen(arquivo, "r"); // r = Read | w = Write | a = Append
	if(!fp)	{
		printf("Erro na abertura do arquivo de entrada.\n\n");
		getchar();
		return 0;
	}
	else{
		i = fgetc(fp);//Pega o caractere e já aponta para o próximo
		tipo++;//Incrementa o tipo de inserção do código
		while(!feof(fp)){
			//Verifica fim da linha
			if((char)i == '\n'){
				//Verifica tipo de entrada do arquivo
				if(tipo == 1){//Linha do nome do curso
					sscanf(x, "%*s %s", nome);
					tipo++;
				}
				else if(tipo == 2){//Linha da quantidade de disciplinas
					sscanf(x, "%*s %d", &disciplinas);
					disc = (Disciplina*) malloc(disciplinas * sizeof(Disciplina));
					prof = (Professores*) malloc(disciplinas * sizeof(Professores));	
					tipo++;
				}
				else if(tipo == 3){//Linha da quantidade de salas
					sscanf(x, "%*s %d", &salas);
					sala = (Sala*) malloc(salas * sizeof(Sala));
					tipo++;
				}
				else if(tipo == 4){//Linha da quantidade de dias
					sscanf(x, "%*s %d", &dias);
					tipo++;
				}
				else if(tipo == 5){//Linha da quantidade de períodos por dia
					sscanf(x, "%*s %d", &periodos_dia);
					tipo++;
				}
				else if(tipo == 6){//Linha da quantidade de cursos
					sscanf(x, "%*s %d", &cursos);
					curso = (Curso*) malloc(cursos * sizeof(Curso));
					tipo++;
				}
				else if(tipo == 7){//Linha da quantidade de restrições
					sscanf(x, "%*s %d", &restricoes);
					restricao = (Restricao*) malloc(restricoes * sizeof(Restricao));
					posicao_restricao[0] = (int*) malloc(disciplinas * sizeof(int));
					posicao_restricao[1] = (int*) malloc(disciplinas * sizeof(int));
					for(i = 0; i < disciplinas; i++){
						posicao_restricao[0][i] = -1;
						posicao_restricao[1][i] = -1;
					}
					tipo++;
				}
				else if(tipo == 10){//Linhas das disciplinas
					if(c < disciplinas){
						sscanf(x, "%*s %s", char_aux);
						aux = numProf(char_aux);
						if(aux == -1){	//Professor ainda não catalogado
							aux = professores;
							sscanf(char_aux, "%s", prof[professores].nome);
							professores++;
						}

						sscanf(x, "%s %*s %d %d %d", disc[c].nome, &disc[c].aulas,
						&disc[c].minDias, &disc[c].alunos);
						disc[c].prof = aux;
						strcpy(disc[c].profe, prof[aux].nome);
						disc[c].cursos = (int*) malloc(cursos * sizeof(int));
						for(i = 0; i < cursos; i++) disc[c].cursos[i] = 0;
						c++;
					}
					else{tipo = 0; c = 0;}
				}
				else if(tipo == 20){//Linhas das salas
					if(c < salas){
						sscanf(x, "%s %d", sala[c].nome, &sala[c].capacidade);
						c++;
					}
					else{tipo = 0; c = 0;}
				}
				else if(tipo == 30){//Linhas dos cursos
					if(c < cursos){
						sscanf(x, "%s %d", curso[c].nome, &curso[c].qtDisc);
						curso[c].disciplina = (int*) malloc(curso[c].qtDisc * sizeof(int));
						aux = 0;
						lc = strtok(x, " ");
						while(lc != NULL){
							if(aux >= 2){	//Começa a listagem das disciplinas
								sscanf(lc, "%s", char_aux);
								curso[c].disciplina[aux - 2] = numDisciplina(char_aux);
								disc[numDisciplina(char_aux)].cursos[c] = 1;
							}
							lc = strtok(NULL, " ");
							aux++;
						}
						c++;
					}
					else{tipo = 0; c = 0;}
				}
				else if(tipo == 40){//Linha das restrições
					if(c < restricoes){
						sscanf(x, "%s %d %d", char_aux, &restricao[c].dia, &restricao[c].per);
						restricao[c].disciplina = numDisciplina(char_aux);
						if(posicao_restricao[0][numDisciplina(char_aux)] == -1)
							posicao_restricao[0][numDisciplina(char_aux)] = c;
						posicao_restricao[1][numDisciplina(char_aux)] = c;
						c++;
					}
					else{tipo = 0; c = 0;}
				}
				else if(strcmp(x, "COURSES:") == 0)	{tipo = 10; c = 0;}
				else if(strcmp(x, "ROOMS:") == 0)	{tipo = 20; c = 0;}
				else if(strcmp(x, "CURRICULA:") == 0)	{tipo = 30; c = 0;}
				else if(strcmp(x, "UNAVAILABILITY_CONSTRAINTS:") == 0){	tipo = 40; c = 0;}
				strcpy(x, "");
			}
			else sprintf(x, "%s%c", x, i); //Acrescenta o caractere na string x
			i = fgetc(fp);//Pega o caractere e já aponta para o próximo
		}
	}
	fclose(fp);

	total_periodos = periodos_dia * dias;

	if((rotina == 0)||(rotina == 4 && programa == 7)){
		aux_mov_r21 = (int*) malloc(disciplinas * sizeof(int));
		aux_mov_r22 = (int*) malloc(disciplinas * sizeof(int));
		aux_mov_r4 = (int*) malloc(disciplinas * sizeof(int));
		aux_mov_r5 = (int*) malloc(disciplinas * sizeof(int));
		aux_mov_r6 = (int**) malloc(total_periodos * sizeof(int *));
		aux_mov_r7 = (int**) malloc(disciplinas * sizeof(int *));
		aux_mov_r8 = (int*) malloc(periodos_dia*dias * sizeof(int));

		r1 =  (int*) malloc(disciplinas * sizeof(int));
		r21 = (int**) malloc(total_periodos * sizeof(int *));
		r22 = (int**) malloc(total_periodos * sizeof(int *));
		r5 = (int**) malloc(disciplinas * sizeof(int *));
		r8 = (int*) malloc(disciplinas * sizeof(int));
		for(i = 0; i < total_periodos; i++){
			r21[i] = (int*) malloc(professores * sizeof(int));
			r22[i] = (int*) malloc(cursos * sizeof(int));
			aux_mov_r6[i] = (int*) malloc(salas * sizeof(int));
		}
		for(i = 0; i < disciplinas; i++){
			r5[i] = (int*) malloc(dias * sizeof(int));
			aux_mov_r7[i] = (int*) malloc(2 * sizeof(int));
		}
	}
	return 1;
}

//Escreve a matriz
void imprimeSolucao(Matriz matriz){
	int i, j;
	printf("\n");
	
	printf("[Dia/Per");
	for(j = 0; j < salas; j++) printf("|%s\t", sala[j].nome);
	printf("|]\n");	
	for(i = 0; i < total_periodos; i++){
		printf("[ %d, %d\t", i/periodos_dia, i%periodos_dia);
		for(j = 0; j < salas; j++){
			if(matriz.n[i][j] < 0) printf("|-(%d)-\t", matriz.n[i][j]);
			else printf("|%s\t", disc[matriz.n[i][j]].nome);
		}
		printf("|]\n");
	}

	printf("\n***FO = %d***\n", matriz.fo);
//	printf("\nTemperatura inicial: %.2f", Tinicial);
//	printf("\nNúmero de iterações por vez: %d", maxIteracoes);
//	printf("\nTemperatura Final: %f", Tfinal);
//	printf("\nTaxa de resfriamento: %f\n", alpha);
}

int restricaoR4(int dis, int per){
	int dia = per/periodos_dia;
	int diaPer = per%periodos_dia;
	int i;

	if(posicao_restricao[0][dis] == -1) return 0;	//A disciplina não possui restrições
	//As restrições estão organizadas por disciplina
	for(i = posicao_restricao[0][dis]; i < posicao_restricao[1][dis]; i++){
		if((restricao[i].disciplina == dis) && (restricao[i].dia == dia) && (restricao[i].per == diaPer))
			return 1;//Retorna 1 em caso de existir a restrição
	}
	return 0;//Retorna 0 no caso de não exista a restrição
}

int restricaoR6(Matriz matriz, int dis, int per, int sal){
	int i, j, penalidade, ok;

	penalidade = 0;
	for(i = 0; i < cursos; i++){
		ok = 0;
		if(disc[dis].cursos[i] == 1){
			for(j = 0; j < salas; j++){
				//if(per < (dia * periodos_dia) - 1){
				if(((per % periodos_dia) < (periodos_dia - 1))&&(ok == 0)){
					if(matriz.n[per + 1][j] != -1){
						if(disc[matriz.n[per + 1][j]].cursos[i] == 1){
							ok = 1; //Encontrou uma disciplina adjacente. Basta.
							j = salas;	//Encerrar o laço
						}
					}
				}
				if(((per % periodos_dia) > 0)&&(ok == 0)){
					if(matriz.n[per - 1][j] != -1){
						if(disc[matriz.n[per - 1][j]].cursos[i] == 1) {
							ok = 1; //Encontrou uma disciplina adjacente. Basta.
							j = salas;	//Encerrar o laço
						}
					}
				}
			}
			if(ok == 0){
				penalidade += 2;
				restricoes_violadas[6]++;
				aux_mov_r6[per][sal] = dis;
			}
			//Não encontrado um adjacente para o curso i
		}
	}
	return penalidade;
}

void setVetor(int *vetor, int tamanho, int valor){
	int i;
	for(i = 0; i < tamanho; i++) vetor[i] = valor;
}

void setMatriz(int **vetor, int linha, int coluna, int valor){
	int i, j;
	for(i = 0; i < linha; i++){
		for(j = 0; j < coluna; j++)vetor[i][j] = valor;
	}
}

int somaVetor(int *vetor, int tamanho){
	int soma, i;
	soma = 0;
	for(i = 0; i < tamanho; i++)soma += vetor[i];
	return soma;
}

//Faz o cálculo da função objetivo
int calcula_FO(Matriz matriz){
	int fo = 0;
	int i, j, k;
	int r5_aux, sub_r7;

	setMatriz(r21, total_periodos, professores, 0);
	setMatriz(r22, total_periodos, cursos, 0);
	setMatriz(r5, disciplinas, dias, 0);
	setMatriz(aux_mov_r6, total_periodos, salas, -1);
	setMatriz(aux_mov_r7, disciplinas, 2, -1);

	setVetor(aux_mov_r21, disciplinas, -1);
	setVetor(aux_mov_r22, disciplinas, -1);
	setVetor(aux_mov_r4, disciplinas, -1);
	setVetor(aux_mov_r5, disciplinas, -1);
	setVetor(aux_mov_r8, total_periodos, -1);
	setVetor(r1, disciplinas, 0);
	setVetor(r8, disciplinas, -1);
	setVetor(restricoes_violadas, 9, -1);

	for(i = 0; i < total_periodos; i++){
		for(j = 0; j < salas; j++){
			if(matriz.n[i][j] != -1){ 
				r1[matriz.n[i][j]]++;
				r21[i][disc[matriz.n[i][j]].prof]++;
				if(r21[i][disc[matriz.n[i][j]].prof] > 1) aux_mov_r21[matriz.n[i][j]] = (j * total_periodos) + i;	//Grava informação para possível troca
				for(k = 0; k < cursos; k++){
					if(disc[matriz.n[i][j]].cursos[k] == 1)	r22[i][k]++;
					if(r22[i][k] > 1) aux_mov_r22[matriz.n[i][j]] = (j * total_periodos) + i;	//Grava informação para possível troca
		
				}
				if(restricaoR4(matriz.n[i][j], i) == 1){
					aux_mov_r4[matriz.n[i][j]] = (j * total_periodos) + i;	//Grava informação para possível troca
					restricoes_violadas[4]++;
					fo += 1000000;//R4 violada (restrição grave!)
				}
				r5[matriz.n[i][j]][i/periodos_dia]++;

				fo += restricaoR6(matriz, matriz.n[i][j], i, j);//R6 violada (restrição leve)
				sub_r7 = disc[matriz.n[i][j]].alunos - sala[j].capacidade;
				if(sub_r7 > 0){
					fo += sub_r7; //R7 violada (restrição leve)
					restricoes_violadas[7]+= sub_r7;
					if(aux_mov_r7[matriz.n[i][j]][0] < sub_r7){
						aux_mov_r7[matriz.n[i][j]][0] = sub_r7;
						aux_mov_r7[matriz.n[i][j]][1] = (j * total_periodos) + i;	//período e sala
					}		
				}
				if(r8[matriz.n[i][j]] == -1) r8[matriz.n[i][j]] = j;
				else if(r8[matriz.n[i][j]] != j){
					fo += 1;	//R8 violada (restrição leve)
					restricoes_violadas[8]++;
					if(restricoes_violadas[8] < total_periodos)// definir um limite de armazenamento
						aux_mov_r8[restricoes_violadas[8]] = (j * total_periodos) + i; //coordenada da disciplina com problema	
				}
			}
		}
	}

	for(i = 0; i < total_periodos; i++){
		for(j = 0; j < professores; j++){
			if(r21[i][j] > 1){
				fo += 1000000 * (r21[i][j] - 1);//R2 violada (restrição grave!)
				if(restricoes_violadas[2] < 0) restricoes_violadas[2] = 1;
				else restricoes_violadas[2] += 1;
			}
		}
		for(j = 0; j < cursos; j++){
			if(r22[i][j] > 1){
				fo += 1000000 * (r22[i][j] - 1);//R2 violada (restrição grave!)	
				if(restricoes_violadas[2] < 0) restricoes_violadas[2] = 1000;
				else restricoes_violadas[2] += 1000;
			}
		}
	}
	for(i = 0; i < disciplinas; i++){
		r5_aux = 0;
		if(r1[i] != disc[i].aulas) fo += 1000000 * (modulo(r1[i], disc[i].aulas));//R1 violada (restrição grave!)
		for(j = 0; j < dias; j++)if(r5[i][j] > 0)r5_aux++;
		if(r5_aux < disc[i].minDias){
			fo += 5 * (disc[i].minDias - r5_aux);//R5 violada (restrição leve)
			restricoes_violadas[5] += r5_aux;
			aux_mov_r5[i] = r5_aux;
		}
	}
	return fo;
}

//Cria uma solução inicial para a meta-heurística
Matriz criaMatriz(){
	Matriz matriz;
	int i, j;

	matriz.n = (int**) malloc(total_periodos * sizeof(int *));
	for(i = 0; i < total_periodos; i++){
		matriz.n[i] = (int*) malloc(salas * sizeof(int));
		for(j = 0; j < salas; j++) matriz.n[i][j] = -1;
	}
	return matriz;
}

//Copia duas variáveis do tipo Matriz
void copiaMatriz(Matriz *destino, Matriz origem){
	int i, j;
	for(i = 0; i < total_periodos; i++){
		for(j = 0; j < salas; j++) destino->n[i][j] = origem.n[i][j];
	}
	destino->fo = origem.fo;
}

//Gera um vizinho aleatório para a solução passada
Matriz geraViz(Matriz matriz){
	int i, j, k, l, busca;
	int per, sal, aux, aux2, aux3, tentativas, movimento;

	aux = -1;
	if(T < 1) tentativas = 6;
	else if(T < 10) tentativas = 5;
	else if(T < 100) tentativas = 4;
	else if(T < 1000) tentativas = 3;
	else tentativas = 2;	
	

	movimento = randomInt(0, 700);
	//Movimento para R2 (aulas conflitantes) a respeito de professores
	if((restricoes_violadas[2] != -1) && (movimento < 100 + ((restricoes_violadas[2]%1000) << 7))){
		if(restricoes_violadas[2] % 1000 > 1){	
			i = 0;
			aux = -1;
			while(aux == -1){
				j = randomInt(0, disciplinas - 1);
				if(aux_mov_r21[j] != -1) aux = j;
				else if(aux_mov_r21[i] != -1) aux = i;
				i++;
			}

			sal = aux_mov_r21[aux] / total_periodos;
			per = aux_mov_r21[aux] - (sal * total_periodos);
			aux2 = -2;
			while(aux2 == -2){
				k = randomInt(0, total_periodos - 1);
				l = randomInt(0, salas - 1);
				if(matriz.n[k][l] == -1){
					aux2 = matriz.n[per][sal];
					matriz.n[per][sal] = -1;
					matriz.n[k][l] = aux2;
				}
				else if(r21[k][disc[matriz.n[k][l]].prof] == 0){//Professor sem problemas
					aux2 = matriz.n[k][l];
					matriz.n[k][l] = matriz.n[per][sal];	
					matriz.n[per][sal] = aux2;
				}
			}
			aux_mov_r21[aux] = -1;
			restricoes_violadas[2]--;
		}
		else{	//troca aleatória igual ao último caso
			aux = randomInt(1, tentativas);
			while(aux > 0){
				i = randomInt(0, total_periodos - 1);
				j = randomInt(0, salas - 1);
				k = randomInt(0, total_periodos - 1);
				l = randomInt(0, salas - 1);
				if((matriz.n[i][j] != -1) || (matriz.n[k][l] != -1)){
					aux2 = matriz.n[i][j];
					matriz.n[i][j] = matriz.n[k][l];
					matriz.n[k][l] = aux2;
					aux--;
				}
			}
		}
	}

	//Movimento para R2 (aulas conflitantes) a respeito de cursos
	else if((restricoes_violadas[2] != -1) && (movimento < 100 + (restricoes_violadas[2] >> 3))){
		if(restricoes_violadas[2] > 1000){
			i = 0;
			aux = -2;
			while(aux == -2){
				j = randomInt(0, disciplinas - 1);
				if(aux_mov_r22[j] != -1) aux = j;
				else if(aux_mov_r22[i] != -1) aux = i;
				i++;
			}
			aux2 = -2;

			sal = aux_mov_r22[aux] / total_periodos;
			per = aux_mov_r22[aux] - (sal * total_periodos);
			while(aux2 == -2){
				k = randomInt(0, total_periodos - 1);
				l = randomInt(0, salas - 1);
				if(matriz.n[k][l] == -1){
					aux2 = matriz.n[per][sal];
					matriz.n[per][sal] = -1;
					matriz.n[k][l] = aux2;
				}
				else if(k != per){	//Ao menos um peŕiodo diferente
					aux2 = matriz.n[k][l];
					matriz.n[k][l] = matriz.n[per][sal];	
					matriz.n[per][sal] = aux2;
				}
			}
			aux_mov_r22[aux] = -1;
			restricoes_violadas[2] -= 1000;
		}
		else{	//troca aleatória igual ao último caso
			aux = randomInt(1, tentativas);
			while(aux > 0){
				i = randomInt(0, total_periodos - 1);
				j = randomInt(0, salas - 1);
				k = randomInt(0, total_periodos - 1);
				l = randomInt(0, salas - 1);
				if((matriz.n[i][j] != -1) || (matriz.n[k][l] != -1)){
					aux2 = matriz.n[i][j];
					matriz.n[i][j] = matriz.n[k][l];
					matriz.n[k][l] = aux2;
					aux--;
				}
			}
		}		
	}

	//Movimento para R6 (Compacidade do currículo)
	else if((restricoes_violadas[6] != -1) && (movimento >= 100) && (movimento < 200 + (2 * restricoes_violadas[6]))){
		int r6_i, r6_j;
		aux = -2;
		k = 0;
		l = 0;
		while(aux == -2){
			i = randomInt(0, total_periodos - 1);
			j = randomInt(0, salas - 1);
			if(aux_mov_r6[i][j] != -1){
				aux = aux_mov_r6[i][j];
				r6_i = i;
				r6_j = j;
			}
			else if(aux_mov_r6[k][l] != -1){
				aux = aux_mov_r6[k][l];
				r6_i = k;
				r6_j = l;
			}
			l = (l+1) % salas;
			if(l == 0) k = (k+1) % total_periodos;
		}
		aux2 = -2;
		aux3 = 0;
		while(aux2 == -2){
			k = randomInt(0, total_periodos - 1);
			l = randomInt(0, salas - 1);
			if((matriz.n[k][l] == -1) && (r6_i != k)){	//Período diferente e vago
				matriz.n[k][l] = matriz.n[r6_i][r6_j];
				matriz.n[r6_i][r6_j] = -1;
				aux2 = 0;
			}
			else if((aux_mov_r6[k][l] != -1) && (r6_i != k)){//Outra disciplina com problemas em outro período
				aux2 = matriz.n[k][l];
				matriz.n[k][l] = matriz.n[r6_i][r6_j];
				matriz.n[r6_i][r6_j] = aux2;
				aux_mov_r6[k][l] = -1;
			}
			else if((aux3 >= tentativas) && (r6_i != k)){//Outro período pelo menos, talvez
				aux2 = matriz.n[k][l];	
				aux = matriz.n[r6_i][r6_j];
				matriz.n[k][l] = aux;	
				matriz.n[r6_i][r6_j] = aux2;
				aux_mov_r6[k][l] = -1;
			}
			else if(aux3 >= tentativas){//Tentou uma troca razoável. Não deu? Vai em qualquer lugar mesmo.
				aux2 = matriz.n[r6_i][r6_j];
				matriz.n[r6_i][r6_j] = matriz.n[k][l];
				matriz.n[k][l] = aux2;
			}
			aux3++;
		}
		restricoes_violadas[6] -= 2;
		aux_mov_r6[r6_i][r6_j] = -1;
	}

	//Movimento para R7 (Capacidade da sala)
	else if((restricoes_violadas[7] != -1) && (movimento >= 200) && (movimento < 300 + restricoes_violadas[7])){
		i = 0;
		j = 0;
		aux = -1;
		while(aux == -1){
			j = randomInt(0, disciplinas - 1);
			if(aux_mov_r7[j][0] != -1) aux = j;
			else if(aux_mov_r7[i][0] != -1) aux = i;
			i++;
		}
		aux2 = -2;
		aux3 = 0;

		sal = aux_mov_r7[aux][1] / total_periodos;
		per = aux_mov_r7[aux][1] - (sal * total_periodos);
		while(aux2 == -2){
			k = randomInt(0, total_periodos - 1);
			l = randomInt(0, salas - 1);
			//Sala vazia e com capacidade suficiente
			if((matriz.n[k][l] == -1) && (sala[l].capacidade >= disc[aux].alunos)){	
				aux2 = matriz.n[per][sal];
				matriz.n[per][sal] = -1;
				matriz.n[k][l] = aux2;
			}
			//Sala não está vazia, mas aloca adequadamente a disciplina com problemas e a que está lá está inadequada
			else if((matriz.n[k][l] != -1) && (sala[l].capacidade >= disc[aux].alunos) && (disc[matriz.n[k][l]].alunos - sala[l].capacidade > aux_mov_r7[aux][0])){
				aux2 = matriz.n[per][sal];
				matriz.n[per][sal] = matriz.n[k][l];
				matriz.n[k][l] = aux2;
			}
			else if((matriz.n[k][l] != -1) && (disc[matriz.n[k][l]].alunos > sala[l].capacidade)){//Disciplina também com
				aux2 = matriz.n[per][sal];
				matriz.n[per][sal] = matriz.n[k][l];
				matriz.n[k][l] = aux2;
			}
			else if((aux3 >= tentativas) && (sala[sal].capacidade > sala[l].capacidade)){//Troca para uma sala maior
				aux2 = matriz.n[per][sal];
				matriz.n[per][sal] = matriz.n[k][l];
				matriz.n[k][l] = aux2;
			}
			else if(aux3 >= tentativas){//Vai qualquer coisa mesmo
				aux2 = matriz.n[per][sal];
				matriz.n[per][sal] = matriz.n[k][l];
				matriz.n[k][l] = aux2;
			}
			aux3++;
		}
		restricoes_violadas[7]--;
		aux_mov_r7[aux][0] = -1;
		aux_mov_r7[aux][1] = -1;
	}

	//Movimento para R8 (Estabilidade de salas)
	else if((restricoes_violadas[8] != -1) && (movimento >= 300) && (movimento < 400 + restricoes_violadas[8])){
		if(restricoes_violadas[8] >= total_periodos) aux = randomInt(0, total_periodos - 1);
		else aux = randomInt(0, restricoes_violadas[8]);
		sal = aux_mov_r8[aux] / total_periodos;
		per = aux_mov_r8[aux] - (sal * total_periodos);
		aux2 = -2;
		aux3 = 0;
		while(aux2 == -2){
			i = randomInt(0, total_periodos - 1);
			j = r8[matriz.n[per][sal]];//Saber a sala da disciplina problemática
			if(matriz.n[i][j] == -1){//Achou vaga na sala "correta" da disciplina
				matriz.n[i][j] = matriz.n[per][sal];
				matriz.n[per][sal] = -1;
				aux2 = 1;
			}
			else if(aux3 >= tentativas){//Vai tentar uma troca decente. Se não, vai em qualquer lugar na coluna
				aux2 = matriz.n[per][sal];
				matriz.n[per][sal] = matriz.n[i][j];
				matriz.n[i][j] = aux2;
			}
			aux3++;
		}
		restricoes_violadas[8]--;
		r8[aux] = -1;
	}

	//Troca aleatória no mesmo período
	else if((movimento >= 400) && (movimento < 500)){
		aux = randomInt(1, tentativas << 1);
		while(aux > 0){
			i = randomInt(0, total_periodos - 1);
			j = randomInt(0, salas - 1);
			//k = randomInt(0, total_periodos - 1);
			l = randomInt(0, salas - 1);
			if((matriz.n[i][j] != -1) || (matriz.n[i][l] != -1)){	
				aux2 = matriz.n[i][j];
				matriz.n[i][j] = matriz.n[i][l];
				matriz.n[i][l] = aux2;
				aux--;
			}
		}
	}

	//Troca aleatória na mesma sala
	else if((movimento >= 500) && (movimento < 600)){
		aux = randomInt(1, tentativas << 1);
		while(aux > 0){
			i = randomInt(0, total_periodos - 1);
			j = randomInt(0, salas - 1);
			k = randomInt(0, total_periodos - 1);
			//l = randomInt(0, salas - 1);
			if((matriz.n[i][j] != -1) || (matriz.n[k][j] != -1)){
				aux2 = matriz.n[i][j];
				matriz.n[i][j] = matriz.n[k][j];
				matriz.n[k][j] = aux2;
				aux--;
			}
		}
	}

	//Troca aleatória total
	else{
		aux = randomInt(1, tentativas);
		while(aux > 0){
			i = randomInt(0, total_periodos - 1);
			j = randomInt(0, salas - 1);
			k = randomInt(0, total_periodos - 1);
			l = randomInt(0, salas - 1);
			if((matriz.n[i][j] != -1) || (matriz.n[k][l] != -1)){
				aux2 = matriz.n[i][j];
				matriz.n[i][j] = matriz.n[k][l];
				matriz.n[k][l] = aux2;
				aux--;
			}
		}
	}
	return matriz;
}

void imprimeTempo(float Tempo, int hora, int minuto){
	if(hora < 10)
		if(minuto < 10)
			if(Tempo < 10)
				printf("\nTempo: 0%d:0%d:0%.3fs", hora, minuto, Tempo);
			else printf("\nTempo: 0%d:0%d:%.3fs", hora, minuto, Tempo);
		else if(Tempo < 10)
				printf("\nTempo: 0%d:%d:0%.3fs", hora, minuto, Tempo);
			else printf("\nTempo: 0%d:%d:%.3fs", hora, minuto, Tempo);
	else if(minuto < 10)
			if(Tempo < 10)
				printf("\nTempo: %d:0%d:0%.3fs", hora, minuto, Tempo);
			else printf("\nTempo: %d:0%d:%.3fs", hora, minuto, Tempo);
		else if(Tempo < 10)
				printf("\nTempo: %d:%d:0%.3fs", hora, minuto, Tempo);
			else printf("\nTempo: %d:%d:%.3fs", hora, minuto, Tempo);
}

void altera_parametros(){

}

Matriz SA(Matriz inicial){
	Matriz atual = criaMatriz();
	Matriz melhor = criaMatriz();
	Matriz viz = criaMatriz();
	//Matriz viz2 = criaMatriz();
	
	clock_t inicio, fim;//Calcular tempo	

	float Tempo, Temp_reaquecimento;
	int i, delta, hora = 0, minuto = 0;
	int reaquecimento = 1;
	int fim_forcado = 0;

	Tinicial = 1000000;
	Tfinal = 0.00001;
	Temp_reaquecimento = Tfinal * 10;

	inicio = clock();

	copiaMatriz(&atual, inicial);
	copiaMatriz(&melhor, atual);

	T = Tinicial;
//1:47
	while ((T > Tfinal) && (melhor.fo > 0) && (fim_forcado < 2000)){
		fim_forcado++;

		if(T > 1000){
			maxIteracoes = 600;
			alpha = 0.98;
		}
		else if(T > 100){
			maxIteracoes = 800;
			alpha = 0.97;
		}
		else if(T > 10){
			maxIteracoes = 1000;
			alpha = 0.98;
		}
		else if(T > 1){
			maxIteracoes = 1200;
			alpha = 0.99;
		}
		else if(T > 0.1){
			maxIteracoes = 1500;
			alpha = 0.993;
		}
		else{
			maxIteracoes = 1200;
			alpha = 0.995;
		}
		for(i = 0; i < maxIteracoes; i++){
			copiaMatriz(&viz, atual);
			if(T < 100)viz.fo = calcula_FO(viz);
			viz = geraViz(viz);
			viz.fo = calcula_FO(viz);
			delta = viz.fo - atual.fo;
			delta = delta << 2;	//Aumentar o fator delta 4 vezes
/*
			if(i == maxIteracoes - 1){
				aux_mat = (aux_mat + 1)%HISTORICO;
				mat_solucao_tempo[aux_mat][0] = melhor.fo;
				mat_solucao_tempo[aux_mat][1] = (clock() - inicio)/ 1000000;
			}
*/
			if(delta < 0){
				copiaMatriz(&atual, viz);
				if(atual.fo < melhor.fo){
					copiaMatriz(&melhor, atual);	
					fim_forcado = 0;
					aux_mat = (aux_mat + 1)%HISTORICO;
					mat_solucao_tempo[aux_mat][0] = melhor.fo;
					mat_solucao_tempo[aux_mat][1] = (clock() - inicio)/ 1000000;
				}			
			}
			else if(randomDouble(0.0, 1.0) < (exp( -1 * (delta / T)))) copiaMatriz(&atual, viz);
		}

		fim = clock();
		Tempo = (double)(fim - inicio)/ 1000000;
		Tempo -= ((hora * 3600) + (minuto * 60));
		if(Tempo >= 60){
			minuto++;
			if(minuto == 60){
				minuto -= 60;
				hora++;
			}
		}
		imprimeTempo(Tempo, hora, minuto);
		if(viz.fo >= 1000000)printf("|  Temp(K) = %.6f \t|  atual.fo = %d \t|  viz.fo = %d\t|  melhor.fo = %d\t (%d)(%d)(%d)", T, atual.fo, viz.fo, melhor.fo, programa, rotina, fim_forcado);
			else printf("|  Temp(K) = %.4f \t|  atual.fo = %d \t|  viz.fo = %d   \t|  melhor.fo = %d\t (%d)(%d)(%d)", T, atual.fo, viz.fo, melhor.fo, programa, rotina, fim_forcado);
		if((T < Temp_reaquecimento) && (reaquecimento > 0)){
			T = Tinicial*0.1;
			reaquecimento--;
		}
		else T *= alpha;
	}
printf("\nT = %.6f, Tfinal = %f, melhor.fo = %d", T, Tfinal, melhor.fo);

	printf("\e[H\e[2J");
	printf("\n");
	imprimeTempo(Tempo, hora, minuto);
	printf("\t| Temp(K) = %.4f \t| FO = %d \t| Melhor FO = %d", T, atual.fo, melhor.fo);
	printf("\nSimulação concluída. Digite o nome do arquivo para gravar os dados.");
	printf("\n\t -> ");	

	return melhor;
}

Matriz solucaoInicial(){
	int i, j, k, cont, atribuicoes;
	Matriz matriz = criaMatriz();

	//Atribuição aleatória
	for(j = 0; j < disciplinas; j++){
		atribuicoes = disc[j].aulas;
		printf("disc[%d].aulas: %d\n", j, disc[j].aulas);
		cont = 0;
		while(atribuicoes > 0){
			i = randomInt(0, total_periodos - 1);
			k = randomInt(0, salas - 1);
			if((matriz.n[i][k] == -1) && (sala[k].capacidade >= disc[j].alunos) && (restricaoR4(j, i) == 0)){
				matriz.n[i][k] = j;
				atribuicoes--;
				cont -= 3;
			}
			else cont++;
			if(cont > 2){
				if(matriz.n[i][k] == -1){
					matriz.n[i][k] = j;
					atribuicoes--;
					cont -= 3;
				}
			}
		}
	}
	matriz.fo = calcula_FO(matriz);
	imprimeSolucao(matriz);
	return matriz;
}

//Exibe o resultado final
void salvaResultado(Matriz matriz, char *str){
	char res[SAIDA];
	strcpy(res, "");

	//scanf("%s", str);//print está em outra função

	FILE *fp = 0;
	if(rotina == 0)fp = fopen(str, "w"); // r = Read | w = Write | a = Append
	else fp = fopen(str, "a"); // r = Read | w = Write | a = Append
	if(!fp){
		printf("ERRO! - Não foi possível salvar os dados.\n");
		return;
	}
	int i, j, k, p, a;//Variaveis de controle

	if(rotina == 0){
		sprintf(res, "Nome: %s\n", nome);
		sprintf(res, "%sDisciplinas: %d\n", res, disciplinas);
		sprintf(res, "%sProfessores: %d\n", res, professores);
		sprintf(res, "%sSalas: %d\n", res, salas);
		sprintf(res, "%sDias: %d\n", res, dias);
		sprintf(res, "%sPeríodos por dia: %d\n", res, periodos_dia);
		sprintf(res, "%sCursos: %d\n", res, cursos);
		sprintf(res, "%sRestrições: %d", res, restricoes);

		for(a = 0; res[a]; a++)	putc(res[a], fp); 
		strcpy(res, "");
		sprintf(res, "\n\nDisciplinas:");
		for(a = 0; res[a]; a++) putc(res[a], fp);
		strcpy(res, "");
		for(i = 0; i < disciplinas; i++){
			sprintf(res, "%s\nDscpl: %s |Prof: %s\t|Aulas: %d\t|MinDias: %d\t|Alunos: %d",
			res, disc[i].nome, disc[i].profe, disc[i].aulas, disc[i].minDias, disc[i].alunos);
			for(a = 0; res[a]; a++) putc(res[a], fp);
			strcpy(res, "");
		}
		sprintf(res, "\n\nSalas:");
		for(a = 0; res[a]; a++) putc(res[a], fp);
		strcpy(res, "");
		for(i = 0; i < salas; i++){
			sprintf(res, "%s\nSala: %s\t|Capacidade: %d", res, sala[i].nome, sala[i].capacidade);
			for(a = 0; res[a]; a++) putc(res[a], fp);
			strcpy(res, "");
		}
		sprintf(res, "\n\nCursos:");
		for(a = 0; res[a]; a++) putc(res[a], fp);
		strcpy(res, "");
		for(i = 0; i < cursos; i++){
			sprintf(res, "%s\nCurso: %s\t|# Dspl: %d", res, curso[i].nome, curso[i].qtDisc);
			for(a = 0; res[a]; a++) putc(res[a], fp);
			strcpy(res, "");
			for(p = 0; p < curso[i].qtDisc; p++){
				sprintf(res, "%s |%s\t", res, disc[curso[i].disciplina[p]].nome);
				for(a = 0; res[a]; a++) putc(res[a], fp);
				strcpy(res, "");
			}
		}
		sprintf(res, "\n\nRestricoes:");
		for(a = 0; res[a]; a++) putc(res[a], fp);
		strcpy(res, "");
		for(i = 0; i < restricoes; i++){
			sprintf(res, "%s\nDspl: %s\t|Dia: %d\t|Período: %d", res, disc[restricao[i].disciplina].nome,
				restricao[i].dia, restricao[i].per);
			for(a = 0; res[a]; a++) putc(res[a], fp);
			strcpy(res, "");
		}
		sprintf(res, "\n\n");
		for(a = 0; res[a]; a++) putc(res[a], fp);

		sprintf(res, "[Dia/Per");
		for(a = 0; res[a]; a++) putc(res[a], fp);
		for(j = 0; j < salas; j++){
			sprintf(res, "|%s\t", sala[j].nome);
			for(a = 0; res[a]; a++) putc(res[a], fp);
		}

		sprintf(res, "|]\n");	
		for(i = 0; i < total_periodos; i++){
			sprintf(res, "%s[ %d, %d\t", res, i/periodos_dia, i%periodos_dia);
			for(j = 0; j < salas; j++){
				if(matriz.n[i][j] < 0) sprintf(res, "%s|-----\t", res);
				else sprintf(res, "%s|%s\t", res, disc[matriz.n[i][j]].nome);
			}
			sprintf(res, "%s|]\n", res);
			for(a = 0; res[a]; a++) putc(res[a], fp);
			strcpy(res, "");
		}
	}

	if(matriz.fo >= 1000000){	//Há ao menos uma restrição grave
		//Restrições R1
		for(i = 0; i < disciplinas; i++){
			if(r1[i] > disc[i].aulas){
				sprintf(res, "\nR1: Disciplina %s (%d) tem mais aulas que o previsto", disc[i].nome, i);
				for(a = 0; res[a]; a++) putc(res[a], fp);
			}
		}
		sprintf(res, "\t");
		putc(res[0], fp);

		//Restrições R2
		for(i = 0; i < total_periodos; i++){
			for(j = 0; j < professores; j++){
				if(r21[i][j] > 1)
					sprintf(res, "\nR2: O professor %s tem mais do que uma aula no mesmo período", prof[j].nome);
					for(a = 0; res[a]; a++) putc(res[a], fp);
			}
			for(k = 0; k < cursos; k++){
				if(r22[i][k] > 1)
					sprintf(res, "\nR2: O curso %s tem mais de uma disciplina no mesmo período", curso[j].nome);
					for(a = 0; res[a]; a++) putc(res[a], fp);
			}
		}
		sprintf(res, "\t");
		putc(res[0], fp);

		//Restrições R4
		for(i = 0; i < total_periodos; i++){
			for(j = 0; j < disciplinas; j++){
				if(matriz.n[i][j] != -1){ 
					if(restricaoR4(matriz.n[i][j], i) == 1){
						sprintf(res, "\nR4: A disciplina %s tem uma restrição no período %d", 
						disc[matriz.n[i][j]].nome, i);
						for(a = 0; res[a]; a++) putc(res[a], fp);
					}
				}
			}
		}
		sprintf(res, "\t");
		putc(res[0], fp);
	}

	//Restrições R5
	int r5_aux;
	for(i = 0; i < disciplinas; i++){
		r5_aux = 0;
		for(j = 0; j < dias; j++)if(r5[i][j] > 0)r5_aux++;
		if(r5_aux < disc[i].minDias){
			sprintf(res, "\nR5: A disciplina %s não possui o número suficiente de dias de trabalho", disc[i].nome);
			for(a = 0; res[a]; a++) putc(res[a], fp);
		}
	}
	sprintf(res, "\t");
	putc(res[0], fp);

	//Restrições R6, R7 e R8
	int r6_aux;
	for(i = 0; i < total_periodos; i++){	//Cálculo das restrições R1, R2 e R4
		for(j = 0; j < salas; j++){
			if(matriz.n[i][j] != -1){ 
				r6_aux = restricaoR6(matriz, matriz.n[i][j], i, j);
				if(r6_aux > 0){
					sprintf(res, "\nR6: A disciplina %s no (dia/per %d/%d, sala %s) encontra-se isolada em %d cursos", disc[matriz.n[i][j]].nome, i/periodos_dia, i%periodos_dia, sala[j].nome, (r6_aux / 2));
					for(a = 0; res[a]; a++) putc(res[a], fp);
				}
				if(sala[j].capacidade < disc[matriz.n[i][j]].alunos){
					sprintf(res, "\nR7: A disciplina %s foi alocada em uma sala com %d a menos de capacidade", disc[matriz.n[i][j]].nome, disc[matriz.n[i][j]].alunos - sala[j].capacidade);
					for(a = 0; res[a]; a++) putc(res[a], fp);
				}
				if(r8[matriz.n[i][j]] == -1) r8[matriz.n[i][j]] = j;
				else if(r8[matriz.n[i][j]] != j){
					sprintf(res, "\nR8: A disciplina %s no (dia/per %d/%d, sala %s) foi alocada em uma sala diferente da inicial", disc[matriz.n[i][j]].nome, i/periodos_dia, i%periodos_dia, sala[j].nome);
					for(a = 0; res[a]; a++) putc(res[a], fp);
				}
			}
		}
	}

	if(rotina == 0)sprintf(res, "\n\nHistórico de busca (tempo em segundos e valor da FO):");
	for(a = 0; res[a]; a++) putc(res[a], fp);
	for(i = 0; i < HISTORICO; i++){
		if(i == 0){
			sprintf(res, "\n\n\n%dº Execução: ***FO = %d***\n", rotina+1, matriz.fo);
			for(a = 0; res[a]; a++) putc(res[a], fp);
		}
		j = mat_solucao_tempo[aux_mat][0];
		k = mat_solucao_tempo[aux_mat][1];
		
		sprintf(res, "\n%dº: %d  %d", i+1, k, j);
		//sprintf(res, "\n%dº FO e tempo em seg: %d  %d", i+1, j, k);
		//sprintf(res, "\n%dº FO:%d em %dh%dmin%dsec\n", i, j, (k/3600), (k/60), k);
		for(a = 0; res[a]; a++) putc(res[a], fp);
		//aux_mat = (aux_mat - 1)%HISTORICO;
		aux_mat--;
		if(aux_mat < 0)aux_mat = HISTORICO - 1;
	}

	//sprintf(res, "\nFim da simulação número: %d\n##########################################################\n\n", rotina+1);
	//for(a = 0; res[a]; a++) putc(res[a], fp);
	fclose(fp);
}

//Funcao main
int main(){
	//char str[SIZE];
	//char str2[SIZE];// = "analise";//Caminho do arquivo de entrada
	char str[SIZE][30];
	char str2[SIZE][30];// = "analise";//Caminho do arquivo de entrada
	Matriz matriz;
	int i;

	srand(time(NULL));
	T = 10000;
	execucao = 0;

	strcpy(str[1], "inst1");strcpy(str2[1], "resultados/inst1_resultado");
	strcpy(str[2], "inst2");strcpy(str2[2], "resultados/inst2");
	strcpy(str[3], "inst3");strcpy(str2[3], "resultados/inst3");
	strcpy(str[4], "inst4");strcpy(str2[4], "resultados/inst4");
	strcpy(str[5], "inst5");strcpy(str2[5], "resultados/inst5");
	strcpy(str[6], "inst6");strcpy(str2[6], "resultados/inst6");
	strcpy(str[7], "inst7");strcpy(str2[7], "resultados/inst7");
	strcpy(str[8], "inst8");strcpy(str2[8], "resultados/inst8");
	strcpy(str[9], "inst9");strcpy(str2[9], "resultados/inst9");
	strcpy(str[10], "inst10");strcpy(str2[10], "resultados/inst10");
	strcpy(str[11], "inst11");strcpy(str2[11], "resultados/inst11");
	strcpy(str[12], "inst12");strcpy(str2[12], "resultados/inst12");
	strcpy(str[13], "inst13");strcpy(str2[13], "resultados/inst13");
	strcpy(str[14], "inst14");strcpy(str2[14], "resultados/inst14");
	strcpy(str[15], "inst15");strcpy(str2[15], "resultados/inst15");
	strcpy(str[16], "inst16");strcpy(str2[16], "resultados/inst16");
	strcpy(str[17], "inst17");strcpy(str2[17], "resultados/inst17");
	strcpy(str[18], "inst18");strcpy(str2[18], "resultados/inst18");
	strcpy(str[19], "inst19");strcpy(str2[19], "resultados/inst19");
	strcpy(str[20], "inst20");strcpy(str2[20], "resultados/inst20");
	strcpy(str[21], "inst21");strcpy(str2[21], "resultados/inst21");

	printf("\e[H\e[2J");	//Limpa o terminal
	programa = 7;
	while(programa <= 21){	
		professores = 0;
		disciplinas = 0;
		salas = 0;
		dias = 0;
		periodos_dia = 0;
		cursos = 0;
		restricoes = 0;	

		printf("\n\n   ----Programação de agendas escolares com Simulated Annealing-----------\n");
		printf("   -	           Digite 1 para resolver um agendamento; 	         -\n");
		printf("   -	           Digite qualquer outro número para sair.	         -\n");
		printf("   -----------------------------------------------------------------------\n");
		printf("\n\n\tOpção: -> ");
		//scanf("%d", &i);

		if(rotina == 0){
			
			printf("\e[H\e[2J");
			printf("\n\n\tInsira um arquivo no formato do ITC 2007");
			printf("\n\n\t--------------------Formato do ITC 2007--------------------");
			printf("\n\tCourses: <CourseID> <Teacher> <# Lectures> <MinWorkingDays> <# Students>");
			printf("\n\tRooms: <RoomID> <Capacity>");
			printf("\n\tCurricula: <CurriculumID> <# Courses> <MemberID> ... <MemberID>");
			printf("\n\tUnavailability_Constraints: <CourseID> <Day> <Day_Period>");
			printf("\n\t--------------------Formato do ITC 2007--------------------");
/*
			printf("\n\n\tArquivo: -> ");
			scanf("%s", str);
			printf("\n\n\tQuantas simulações serão realizadas?: -> ");
			scanf("%d", &num_exec);			
			//strcpy(str, "teste");	
			printf("\n\n\tNome do arquivo a serem salvos os resultados: -> ");
			scanf("%s", str2);
*/	
		}
		else if(rotina < num_exec);//Não faz nada além de pular o cabeçalho		
		if(!leArquivos(str[programa]))
			printf("\n\nERRO! - Houve um problema para ler o arquivo. Tente novamente\n\n");	
		else{		
			printf("\e[H\e[2J");
			matriz = solucaoInicial();
			matriz = SA(matriz);
			calcula_FO(matriz);//Atualizar o valor das variáveis auxiliares
			imprimeSolucao(matriz);
			salvaResultado(matriz, str2[programa]);
			printf("\e[H\e[2J");
			printf("\nArquivo %s com as informações criado com sucesso.\n", nome);
			rotina++;
			if(rotina == num_exec){
					for(i = 0; i < total_periodos; i++){
						//free(r21[i]);
						free(r22[i]);
						free(aux_mov_r6[i]);
					}
					for(i = 0; i < disciplinas; i++){
						free(r5[i]);
						free(aux_mov_r7[i]);
					}
					free(aux_mov_r21);
					free(aux_mov_r22);
					free(aux_mov_r4);
					free(aux_mov_r5);
					free(aux_mov_r6);
					free(aux_mov_r7);
					free(aux_mov_r8);
					free(r1);
					free(r21);
					free(r22);
					free(r5);
					//free(r8);
				
				programa++;
				rotina = 0;
				num_exec = 12;
			}
		}
	}
	printf("\e[H\e[2J");
	return 0;
}

