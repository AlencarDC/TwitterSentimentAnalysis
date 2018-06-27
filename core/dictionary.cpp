#include "dictionary.h"
#include <stdlib.h>
#include <cmath>

#define SEED 7
#define SEED2 3
#define MAXLENGHT 8
#define TAXAMAXIMA 0.5
#define EXTRASIZE 10
#define INITSIZE 11

//Constructor for dictionary
Dictionary::Dictionary() {
    currentSize = 0;                    //Set the current size to 0 and then resizes the
    setMaxSize(nextPrime(INITSIZE));    //vector to INITSIZE.

}

//Acumulação polinomial
int Dictionary::hash1 (utf8_string key) {
    unsigned int hashValue = 0;

    for(int i = 0; i < key.size() && i < MAXLENGHT; i++) {
        hashValue = (hashValue * SEED) + key.at(i);
    }
    return hashValue % maxSize;

}

int Dictionary::hash2 (utf8_string key) {
    unsigned int hashValue = 0;

    for(int i = 0; i < key.size() && i < MAXLENGHT; i++) {
        hashValue = (hashValue * SEED2) + key.at(i);
    }
    return (hashValue % SEED) + 1;
}

//CHanges max size
void Dictionary::setMaxSize(int newSize) {
    maxSize = newSize;
    table.resize(newSize);
}

//Insere uma palavra
int Dictionary::insertWord(wordData newWord) {
    unsigned int key;
    unsigned int firstKey  = hash1(newWord.word);
    unsigned int secondKey = hash2(newWord.word);
    int j = 0;

    currentSize++;

    // Get an empty position in the table
    if(needReHash()){
        resizeDictionary();
    }

    // Get an empty position in the table
    do {
        key = (firstKey + j * secondKey) % maxSize;
        j++;
    } while (isEmpty(key) == false);

    table[key] = newWord;
    return key;
}
//Retorna uma palavra
wordData Dictionary::retrieveWordData(utf8_string word) {
    int key;
    wordData emptyWord;

    key = find(word);

    if (key == -1)  //If word given isnt in the table return a empty struct
        return emptyWord;

    return table[key];
}

bool Dictionary::isEmpty(unsigned int key) {
    return table[key].word.empty();
}

int Dictionary::find(utf8_string word) {
    unsigned int key;
    unsigned int firstKey  = hash1(word);
    unsigned int secondKey = hash2(word);
    std::string wordBuff;
    std::string wordCopy = word.c_str();
    int j = 0;

    do {    //Calculate key searching for the word
        key = (firstKey + j * secondKey) % maxSize;
        wordBuff = table[key].word.c_str();
        j++; //Now it increments on while statement
    } while ((wordBuff != wordCopy) && (key != firstKey || j-1 == 0));

    if (wordBuff != wordCopy) //word isnt in the dictionary
        return -1;

    return key;
}

bool Dictionary::isPrime(int number){
        for (int div = 2; div < number; div++){
            if (!(number % div)){
                return false;
            }
        }
    return true;
}


int Dictionary::nextPrime(int actualNumber){
    actualNumber++;
    while(!isPrime(actualNumber)){
        actualNumber++;
    }

    return actualNumber;
}

bool Dictionary::needReHash(){
    float ocupacao;

    ocupacao = (float) currentSize / (float) maxSize;

    if(ocupacao < TAXAMAXIMA){
        return false;
    }
    else{
        return true;
    }
}

//Re-hash funcion
void Dictionary::resizeDictionary(){
    int newSize;

    std::vector <STATUS> controle;

    //Calcula novo tamanho e atualiza tamanho da tabela
    newSize = maxSize + EXTRASIZE;
    newSize = nextPrime(newSize);
    setMaxSize(newSize);

    controle.resize(newSize);
    //Prepara vetor que realiza o controle das realocações
    for(int i = 0; i < controle.size(); i++){
        if (!isEmpty(i)){
            controle[i].livre = false;
            controle[i].ocupado = true;
        }
    }

    //Realoca toda tabela. (se a posicao é livre a função já identifica isso e não faz nada)
    //então não é necessario duplicar o teste aqui
    for (int i = 0; i < maxSize; i++){
        //std::cout << i << std::endl;
        realocaPosicao(i,controle);
    }
}

void Dictionary::realocaPosicao(int posicao, std::vector <STATUS>& controle){
    utf8_string chave;
    wordData buffer;
    int returnedHash;
    //Vê se há algo a ser realocado, se  não tem, retorna
    if (controle[posicao].realocado == true || controle[posicao].livre == true)
        return;
    else{
        buffer = table[posicao];    //Pega wordData da posicao a ser realocada;
        returnedHash = hash1(buffer.word);  //Calcula nova posicao (incremento 0)
        controle[posicao].livre = true;     //libera posicao
        if(controle[returnedHash].livre == true){   //Se nova posicao é livre, insere
            table[returnedHash] = buffer;
            controle[returnedHash].livre = false;   //Muda os status
            controle[returnedHash].ocupado = false;
            controle[returnedHash].realocado = true;
        }

        else if(controle[returnedHash].ocupado == true){
            realocaPosicao(returnedHash,controle);  //Se esta ocupado, realoca a posicao ocupada
            realocaPosicao(posicao,controle);       //E tenta realocar a posicao atual (pode cair em um realocado)
        }

        else{   //Se chegou até aqui a posicao so pode estar realocada, então realiza uma até achar uma posição não realocada
            int j = 1;
            while(controle[returnedHash].realocado == true){
                returnedHash = ( hash1(buffer.word) + (j * hash2(buffer.word)) ) % maxSize;
                j++;
            }
            realocaPosicao(returnedHash,controle);  //Realoca posição achada(pode estar ocupada ou livre)
            realocaPosicao(posicao, controle);      //Tenta inserir na posicao dada novamente
        }
    }
}
