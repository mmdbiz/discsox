<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "welcome";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();
global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$title = "";
$news = "";

ob_start();
if(@include("http://www.quikstore.com/30news.html")){
	$newsPage = ob_get_contents();
	ob_end_clean();
	list($title,$news) = $_Template->getPageBody($newsPage);
}
else{
	ob_end_clean();	
}
?>

<html>
<head>
<title>welcome...</title>
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
	if(fileName != "summary.php"){
		parent.menu.location = 'summary.php';
	}
}
    
    
</script>
</head>
<body>
<div align="center">
<?php if($news != ""):?>
<?=$news;?>
<?php else:?>
<h4 align="center">Welcome <?=$_SESSION['welcome_name'];?></h4>
<p>Select a function from the menu to get started...</p>
<?php endif;?>
</div>
</body>
</html>






