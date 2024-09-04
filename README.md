# AutoGrader
A robust C program designed to automate the grading process of student assignments.

This tool compiles, runs, and evaluates C programs using system calls such as `fork`, `execvp`, `dup`, `open`, and more. 
It compares the program output against a reference solution, grading based on correctness and performance.
The AutoGrader efficiently handles multiple student submissions, ensuring consistency and fairness in evaluation.

## How to Run
1. **Clone the Repository:**
   ```bash
   git clone https://github.com/tomerp1812/AutoGrader.git
   cd AutoGrader
   ```
2. **Compile the Program:**
   ```bash
   gcc AutoGrader.c -o AutoGrader
   ```
3. **Run the Program:**
   ```bash
   ./AutoGrader config.txt
   ```
   - Replace `config.txt` with the path to your configuration file.
     This file should contain the paths for the student directories, input file, and expected output file.
4. **Configuration File Format:**
   - Line 1: Path to the directory containing student submissions.
   - Line 2: Path to the input file.
   - Line 3: Path to the expected output file.
5. **Grading Results:**
   The program generates a `results.csv` file, which contains the grades for each student based on the compilation, execution, and correctness of their programs. Errors and logs are written to `errors.txt`.
