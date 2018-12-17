# Parallel-Programming-Project-Universal-Decision-Tree-Classifier
A universal decision tree classifier for most datasets in UCI

Each branch stands for a parallel programming model.
If the name of branch consists of "single", means that we only construct one decision instead of a random forest.

# How to run?
If you are using a UNIX-like system, you can run our code simply by running "build.sh" shell script with all the source code under the same directory.
That shell script will generate a executable file named "main.out" with one parameter, that is, the name of dataset.

Here's the example of running our model with iris dataset.
```
./build.sh %% ./main.out iris.data
```
