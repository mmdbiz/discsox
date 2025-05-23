<?php
$_isAdmin = true;
$_adminFunction = "news";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

/* News from QuikStore site */

?>