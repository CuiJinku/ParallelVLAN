serial:
	 g++ -fopenmp serial.cpp Chromosome.cpp -o serial
	 g++ -fopenmp openmp.cpp Chromosome.cpp -o openmp
clean:
	rm serial openmp
