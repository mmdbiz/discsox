<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// login is required
$_CF['login']['require_login'] = true;
$login = $_Registry->LoadClass('login');
$login->checkLogin();

$favorites = $_Registry->LoadClass("favorites");

$RUN = false;
foreach($_REQUEST as $key=>$value){
	switch($key){
		case "add":
			$favorites->add();
			$RUN = true;
			break;
		case "delete":
			$favorites->delete();
			$RUN = true;
			break;
		case "display":
			$favorites->display();
			$RUN = true;
			break;
	}
}
if(!$RUN){
	$favorites->display();
}
?>