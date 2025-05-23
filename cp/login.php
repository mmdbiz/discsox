<?php
error_reporting(E_ALL);
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "login";
$_CF = array();
$_CART = array();
$_STRINGS = array();

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

$_SESSION['logging_in_from'] = "welcome.php";

global $_Registry;
if(!empty($_REQUEST['login'])){
	$login = $_Registry->LoadClass("admin_login");
	$login->CheckLogin();
}
if(!empty($_REQUEST['logout'])){
	$login = $_Registry->LoadClass("admin_login");
	$login->logout();
}

?>
<html>
<head>
<title>login</title>
<meta http-equiv="content-type" content="text/html;charset=iso-8859-1">
<script language="JavaScript">
//<!--
if(!eval(parent.menu)) {
    top.parent.content.location.href = "login.php";
}
else{
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "login.menu.html"){
		parent.menu.location = 'menus/login.menu.html';
	}
}
//-->
</script>
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

function checkEntries(form){
     if(form.user.value == "" || form.pass.value == ""){
          alert("You did not enter a valid user name or password?");
          form.user.focus();
          return false;
     }
return true;
}
//-->
</script>
</head>
<body>
<div align="center">
<br><br>
<h4>Please Login</h4>
<form method="post" action="login.php" onSubmit="return checkEntries(this);">
<table border=0 cellspacing=0 cellpadding=2>
	<tr>
		<td width="30%" align="right"><b>User Name:</b></td>
		<td align="left" valign="middle" width="70%">
			<div align="center">
				<input type="text" name="user" size="24" tabindex="1" border="0">
               </div>
		</td>
     </tr>
     <tr>
		<td width="30%" align="right"><b>Password:</b></td>
		<td align="left" valign="middle" width="70%">
			<div align="center">
				<input type="password" name="pass" size="24" tabindex="2" border="0">
               </div>
		</td>
     </tr>
     <tr>
		<td colspan="2" align="center"><br>
				<hr size="1" noshade width="200">
		</td>
	</tr>
     <tr>
		<td colspan="2" align="center">
				<input type="submit" name="login" value="Login">
		</td>
	</tr>
</table>
</form>
</div>
<p></p>
<script language="JavaScript">
	document.forms[0].user.focus();
</script>
</body>
</html>
