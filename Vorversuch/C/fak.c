#include<stdio.h>

unsigned int fak(unsigned int);

int main(){

	int n;
	printf("Please enter a positive Integer!");
	scanf("%d",&n);
	if(n<0){
		printf("Must be a positive Integer!");
		return -1;
	}
	printf("%d\n",fak((unsigned int)n));
}

unsigned int fak(unsigned int n){
	if(n == 0){
		return 1;
	}
	
	int ret = n--;
	while(n>0){
		ret *= n--; 
	}
	return ret;
}
