# Mongoose User Guide

Mongoose is small and easy to use web server built on top of
mongoose library. It is designed with maximum simplicity in mind. For example,
to share any directory, just drop mongoose executable in that directory,
double-click it (on UNIX, run it from shell) and launch a browser at
[http://localhost:8080](http://localhost:8080) Note that 'localhost' should
be changed to a machine's name if a folder is accessed from other computer.

#Features

@CGI, SSI, SSL, Digest auth, Websocket, WEbDAV, Resumed download, URL rewrite, file blacklist
@Custom error pages, Virtual hosts, IP-based ACL, Windows service, HTTP/HTTPS client
@Simple and clean embedding API. The source is in single mongoose.c file to make embedding easy
@Extremely lightweight, has a core of under 40kB and tiny runtime footprint
@Asynchronous, non-blocking core supporting single- or multi-threaded usage
@On the market since 2004 with over 1 million cumulative downloads
@Stable, mature and tested, has several man-years invested in continuous improvement and refinement


# usage

Mongoose version 5.6 (c) Sergey Lyubka, built on Apr  3 2015
Usage:
  mongoose -A <htpasswd_file> <realm> <user> <passwd>
  mongoose [config_file]
  mongoose [-option value ...]

OPTIONS:
  -access_control_list <empty>
  -access_log_file <empty>
  -auth_domain mydomain.com
  -dav_auth_file <empty>
  -dav_root <empty>
  -document_root <empty>
  -enable_directory_listing yes
  -enable_proxy <empty>
  -extra_mime_types <empty>
  -global_auth_file <empty>
  -hide_files_patterns <empty>
  -hexdump_file <empty>
  -index_files index.html,index.htm,index.shtml,index.cgi,index.php
  -listening_port <empty>
  -run_as_user <empty>
  -ssi_pattern **.shtml$|**.shtm$
  -url_rewrites <empty>

Reference: 

Configuration file is a sequence of lines, each line containing
command line argument name and it's value. Empty lines and lines beginning
with `#` are ignored. Here is the example of `mongoose.conf` file:

    # This is a comment
    document_root /etc/www
    listening_port 80
    ssl_certificate /etc/mongoose/cert.pem

# Source Code Download
  https://github.com/cesanta/mongoose
  There are lots of examples

##############################
