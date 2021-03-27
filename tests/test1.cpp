//By Philip Ross
//
//Test 1 Code For Clang C++ Style Checker-code designed to check naming practices


int Foo(int In);

int main(){

	int q = 0;
	int fifteencharactersisalotofcharacters=7;
	float has10lO = 15; //this is a comment next to a float
	char Caps = 'C'; const double pi = 3.14159;
	Caps = 'h';
	q=5;q++; q = 2*pi;
	for(int i=0;i<10;i++){
		q++;
		Caps = pi*3;
		int IsProblem = pi*i;
		IsProblem++;
	}return 0;
}

int Foo(int In){int out;
	out = In*In;
	return out;}
