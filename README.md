# SPASuite

Student-project allocation with Lecturer Preferences on Students With Ties (SPA-ST) is a two-sided matching problem that aims to allocate students to projects based on the preferences of students on the project set provided and the preferences of the academic supervisors on the students applying to their projects. A high-quality matching will be at least weakly-stable, which means that no blocking pairs are admitted. A blocking pair is a student-project pair where student and the project's supervisor have alternative projects or students that are of greater preference to each other - both sides are mutually unsatisfied. In SPA-ST, there may be multiple weakly stable matchings - but obtaining the maximum-size weakly-stable matching is NP-hard.

SPASuite is a [Qt](https://www.qt.io/) application that runs a genetic algorithm in order to find this maximum-size weakly-stable matching. The GA itself is encased within a helpful user interface that allows easy variance of GA parameters, display of GA results, including properties of the best matching found, as well as a comprehensive SPA-ST dataset generator. This GA performs at least as well as [Cooper's 3/2-approximation algorithm](https://github.com/fmcooper/stable-SPA) in all tested instances.

## Building

This project was developed with Qt6 and requires at least CMake 3.5. I recommend loading the project within Qt Creator.

## File formatting

Input files to SPASuite should be .txt files and formatted as follows:

(number of students) (number of supervisors) (number of projects)<br>
For each student, line-by-line:<br>
(student id) (nth choice project id):(n)<br>
For each supervisor, line-by-line:<br>
(supervisor id):(supervisor capacity) (nth choice student id):(n)<br>
For each project, line-by-line:<br>
(project id):(project capacity) (supervisor id)
