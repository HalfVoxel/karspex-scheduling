
# Kårspex Scheduling

Takes as input a list of people and which groups they belong to, as well as a list of tasks/events that they together need to participate in and calculates a somewhat good allocation of people to tasks.
A single person can be allocated to multiple tasks.
The tasks can be constrained in the following ways:

 - An event/task has a specific number of people that need to participate in it.
 - A task can be constrained so that at least one person with a specific group needs to be allocated to it.
 - A task can be constrained so that none of the people in a specific group can be allocated to it.
 - A task can be 'hard', which means that if a person is allocated a hard task, it will count as an additional work burden, and fewer other tasks will be allocated to that person.
 - A pairwise constraint can be added to indicate that no people allocated to a particular task is allowed to be allocated to another specified task as well (e.g. because they happen at the same time).

## Input

The program takes input from stdin (though usually a file is piped to it) on the following format.
The input contains 3 sections, separated by a blank line.
A full input example can be found in the file [example.txt](example.txt).

### People
```
A list of lines on the format:
<comma separated list of groups the person is part of><tab><additional comma separated list of groups the person is part of><tab><first name of person><tab><last name of person, or initial of last name>
```

For example:
```
Produktionschef,Ljus(A)	Scengruppen	Anna	A
Skådis	Scengruppen	Bengt	B
	Revisor	Caroline 	C
```

Since it is likely the user is writing the input using a spreadsheet program. Here is a table view of the same thing.
Note however that the headers should **not** be included in the input data.

| Groups                  | Groups      | First Name | Last Name  |
| ----------------------- | ----------- | ---------- | ---------- |
| Produktionschef,Ljus(A) | Scengruppen | Anna       | A          |
| Skådis                  | Scengruppen | Bengt      | B          |
|                         | Revisor     | Caroline   | C          |

Q: Why are there two comma separated list of groups?
A: Simply to allow copy-pasting from an existing spreadsheet. They are completely identical.

Note that group names cannot contain any whitespace, thus you should for example use "Ljus(A)" and not "Ljus (A)".

### Tasks

A list of lines on the format:
```
<name of task><tab><number of people that should be allocated to the task><tab><space separated list of constraints>

where a constraint is on one of the formats:
hard
	indicates that the task is hard
>groupname
	indicates that at least one person from the group 'groupname' should be allocated to this task
!groupname
	indicates that no people from the group 'groupname' should be allocated to this task
```

For example

```
Checkis in	8	>KMP >Orkestern !Scengruppen !Ljushuvud
Premiär ut campus	12	hard >KMP >Orkestern
```

or as a table (remember headers are not included in the input data)

Task name         | Number of people | Constraints
----------------- | ---------------- | -----------
Checkis in        | 8                | >KMP >Orkestern !Scengruppen !Ljushuvud
Premiär ut campus | 12               | hard >KMP >Orkestern

### Pairwise constraints

Use these constraints to prevent any person that works on a particular task from also working on another task.
Usually these are distinct tasks that happen at the same time, but in different locations. Most people cannot be in multiple places at the same time.

A list of lines on the format:
```
taskname1<tab>taskname2
```

For example
```
Premiär ut teatern	Premiär ut campus
Omg 2 ut teatern	Omg 2 ut campus
```

A full input example can be found in the file [example.txt](example.txt).

### Building

To build the program you need to have a c++ compiler installed with C++11 support. For example any Ubuntu computer at KTH.

```
g++ -std=c++11 -Wall -O2 solve.cpp
```

### Running

To run the program, use a terminal and point the program to the input file.

```
./a.out < example.txt
```

Note that the program is non-determinstic, so running it multiple times will yield different outputs. Usually they are about equally good.
For hard inputs, the program may hang indefinitely if it cannot find a solution. Restarting the program multiple times to allow it to start from different random starting points may allow it to find a solution in those cases.

Internally the very very high level description of what the program does is that it takes the person with the currently lowest work burden and assigns it to the task that needs that person the most. It repeats this step until all tasks have enough people allocated to them.
