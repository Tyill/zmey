<div align="left">
  <a><img src="docs/label.svg" width = 50 height = 50 ></a><br>
</div>

The Zmey software for schedule and monitor workflows.

<p float="left">
<img src="https://github.com/Tyill/zmey/blob/ver_1_0_docs/docs/demo.gif" width="800" height="450"/>
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

## How usage
 - install [PostgreSQL](https://www.postgresql.org/download/) and create DB
 - install Web server: `pip install zmeyweb`
 - run web server (on debug Flask for example): `set FLASK_APP=./server&& flask run --no-reload`
 - usage [config file](https://github.com/Tyill/zmey/blob/master/web/zmserver.cng) with params
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
