<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "utilities";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

?>
<html>
<head>
<title>Utilities...</title>
<script language="JavaScript">
//<!--
    var sHeight = screen.height;
    var sWidth = screen.width;
    var styles = "admin.800.css";
    if(sWidth > 800){
        styles = "admin.1024.css";
    }
    if(sWidth > 1024){
        styles = "admin.1152.css";
    }
    if(sWidth > 1100){
        styles = "admin.1280.css";
    }
    document.write('<link rel="stylesheet" href="stylesheets/' + styles + '" type="text/css">');
    
if(eval(parent.menu)) {
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "utilities.menu.php"){
		parent.menu.location = 'menus/utilities.menu.php';
	}
}
</script>
</head>
<body>
	<div align="center">
		<h4 align="center">Utilities</h4>
		<p>Select a function from the menu to get started...</p>
	</div>
</body>
</html>