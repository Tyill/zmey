<div align="left">
  <a><img src="docs/label.svg" width = 50 height = 50 ></a><br>
</div>

<br/>
The Zmey software for schedule and monitor workflows.
<br/> <br/>
<p float="left">
<img src="docs/demo.gif" width="600" height="350"/>
</p>

## Features

* Multi-user system, each user has his own independent task pool
* Run multiple tasks in parallel on multiple machines
* Linking tasks into chains and graphs for sequential execution
* Automatic transmission of the task execution result for the following tasks
* Automatic start of tasks by time (cyclically or at a specific time)
* Minimum launch delay of a task ready to start (within the time of the database operation and network delay)
* Quick deployment of the system:
  * Installing schedulers and executors on working machines from standard packages (deb, rpm), or docker image;
  * Installing a web server using pip (python package manager)
* Unlimited (logically) number of workers and users of the system
* Simple and intuitive user interface
* MIT Licensed

## Architecture 
<p float="left">
 <img src="docs/functional_scheme.png" 
  width="400" height="395" alt="lorem">
</p>
The task queue is based on the PostgreSQL DB. <br/>
Schedulers periodically poll the database to receive new tasks (using the 'listen-notify' mechanism built into the database). <br/>
Each scheduler has its own pull workers, and when a new task is received, a free worker is selected. <br/>
The maximum number of tasks that a worker can take on is set when the worker is added to the database. <br/>
The worker does not have access to the database, all the results of task execution are transferred to the database by the scheduler. <br/>
Schedulers and workers are created on the administrator page in the web user interface. <br/>
and can be run on any machine on the network. <br/>
<br/>
The web server is built on the Flask micro-framework. <br/>
A local SQLite database is created for each user to store the state of the interface. <br/>
<br/>
The web client is React application, Mobx is used for control of state, Boostrap design.

## How usage
 - install [PostgreSQL](https://www.postgresql.org/download/) and create DB
 - install Web server: `pip install zmeyweb`
 - run web server (on debug Flask for example): `set FLASK_APP=./server&& flask run --no-reload`, <br/> usage [config file](https://github.com/Tyill/zmey/blob/master/web/zmserver.cng) with params:
   - `DbConnectStr` - connection string for DB PostgreSQL
   - `PostgreLibPath` - path to lib `libpq.dll` (actual for Windows)
   - `CoreLibPath` - path to client lib `zmclient.dll`
 - open the page in the browser `http://127.0.0.1:5000/`
 - go to admin page, to do this, set a username `admin` and password `password`
 - append `sheduler` and `worker` process
 - run `scheduler` and `worker` process (pre-build for [Win](https://github.com/Tyill/zmey/tree/master/pre-build/win64) and for [Linux](https://github.com/Tyill/zmey/tree/master/pre-build/deb))
 - go to login page, create user and log on
 

## [Docs (on development stage)](https://tyill.github.io/zmey) 

## License
Licensed under an [MIT-2.0]-[license](LICENSE).
