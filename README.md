<font size="10">Design Plan</font>  
The plan is to use a modular design that allows  code reuse and the ability to add additional features as  
needed. I started by making a main thread that forks to provide the producers of our two products. The main  
thread then continues to redirect standard output to a file. From here the threads are created for the  
consumers, and we use the main thread as the distributor to maximize our efficiency. The producers create  
their products in a loop with the current count/type, while writing to the pipe. After printing their  
required number of product, they send one final product with count of -1 to let the Distributor know  
that it is finished.  
  
The Distributor then takes these values and starts putting them in our buffer using the "enQueue" call,  
which is thread safe. This happens until the Distributor receives a product with count of '-1', which again  
is the sentinel value to indicate the final value for this product type. From there, we mark that we've  
encountered one sentinel (for each type) and send the sentinel value to the consumer threads.  
  
The Consumer threads then 'deQueue' from the buffer, write the desired values to file (using another lock),  
and continue, while using condition variables and mutex to handle synchronous operations. This occurs until  
we encounter the sentinel value entered by the distributor, at which point we use a flag inside our  
consumerBundle struct to notify the other thread that we have completed consumption for this type, and  
signal them to awaken (if necessary).  
  
The synchronous operations were handled using Mutex designated for each type and our file. By having a  
single struct for each type, containing our queue, locks, consumption number, and flag, we allowed for  
easy signalling between threads of each type. Conditional variables were set to trigger producers/  
consumers depending on the size variable located inside our queues. The consumer would trigger a  
signal after consuming from a buffer, and the Distributor would trigger a signal after adding  
to the queue. This way, if there was something to be done they were both doing it. This could  
have been done differently. If we chose to only signal the Distributor when the queue was empty,  
(and only signal the Consumer when full) perhaps we would have different complexity in switching  
between the operations. 

<font size="10">Testing</font>  
There wasn't a whole lot of testing to be done (as you said in lab), but I did run the program  
a lot to make sure the output was consistent. There was also a point where I was worried about  
whether the threads were signalling each other properly and introduced a sleep timer in the  
consumer. This caused a segmentation fault, which lead me to find some bugs. I also noticed  
that the output was being intertwined before there was a file lock, which lead to the creation  
of the file lock of course.

<font size="4">Multiple tests were included in the 'tests' folder</font>