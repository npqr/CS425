# CS425 A4: Routing Protocols (DVR and LSR)

## Compilation & Running the Code

- It is recommended to run this project on a Linux machine.
- Ensure that you have `g++` installed to compile the code.

### Compiling the Code
Run the `make` command to compile the code. This will create an executable named `routing_sim`:

```bash
make all
```

### Running the Code
1. **Launch the Program:**
   Execute the compiled program with the input file as an argument:
   ```bash
   ./routing_sim <input_file>
   ```

   Replace `<input_file>` with the path to one of the provided input files (e.g., input1.txt, input2.txt, etc.).

2. **Expected Behavior:**
   - The program will first simulate the Distance Vector Routing (DVR) algorithm and print the routing tables for each node at each iteration.
   - Then, it will simulate the Link State Routing (LSR) algorithm and print the final routing tables for each node.

3. **Example:**
   ```bash
   ./routing_sim input1.txt
   ```

   The output will display the routing tables for both DVR and LSR simulations.

### Cleaning Up
To remove the compiled executable, run:
```bash
make clean
```

## Design Decisions

- **Distance Vector Routing (DVR):**
  - Each node maintains a routing table with the cost and next hop for all destinations.
  - The algorithm iteratively updates routing tables until no further updates are required.

- **Link State Routing (LSR):**
  - Each node computes the shortest paths to all other nodes using Dijkstra's algorithm.
  - The program outputs the routing table for each node after the computation.

- **Error Handling:**
  - The program exits with an error message if the input file cannot be opened.

## Team Members

- Khushi Gupta (220531)
- Nevish Pathe (220757)

## Declaration

We declare that we did not indulge in plagiarism and the work submitted is our own.
