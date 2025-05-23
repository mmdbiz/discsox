<?php
$_isAdmin = true;
$_adminFunction = "configuration";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

?>


<html>
<head>
<title>Welcome</title>
<script language="JavaScript">
//<!--
if(!eval(parent.menu)) {
    top.parent.content.location.href = "config.welcome.php";
}
else{
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "configuration.php"){
		parent.menu.location = 'configuration.php?displayMenu=true';
	}
}
//-->
</script>
<script LANGUAGE="JavaScript">
//<!--
sWidth = screen.width;
var styles = "admin.800.css";
if(sWidth > 850){
    styles = "admin.1024.css";
}
if(sWidth > 1024){
    styles = "admin.1152.css";
}
if(sWidth > 1100){
    styles = "admin.1280.css";
}
document.write('<link rel="stylesheet" href="stylesheets/' + styles + '" type="text/css">');
if(screen.width < 800){
    alert("The screen resolution for the Online Catalog Builder must be set to a minimum of 800 x 600 for all the navigational options to display properly.");
}
//-->
</script>
</head>
<body id="bdy" class="mainBody">

	<table border="0" width="600" cellspacing="0" cellpadding="3" align="center">
	<tr>
		<td align="center"><h4 align="center">Configuration Editor</h4></td>
	</tr>
	<tr>
		<td align="left" style="font-size:12px;">
		This section provides a way for you to edit all of the general store settings.
		Each section has detailed instructions for each setting that explains what
	    it does. There's also several different general configurations and payment gateway
	    configurations to choose from. Select the specific config to load the available
	    sections for that configuration. Click a link in the menu on the left to begin.
	   </td>
	</tr>

</table>
</body>
</html>