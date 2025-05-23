<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

// Search reports directory for .php files and display as reports
$path = "../reports";
$reportList = array();
$handle = opendir($path);
while ($file = readdir($handle)) {
    if(is_file("$path/$file") && stristr($file,".php") && !stristr($file,".php.LCK") && $file != "invoice.php") {
        $names = explode('.',$file);
        //lose the extension
        array_pop($names);
        $reportName = join(" ", $names);
        $reportList[$file] = ucwords($reportName);
    }
}
ksort($reportList);
?>

<html>
<head>
<title>Report Menu</title>
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
// document.write('<link rel="stylesheet" href="../stylesheets/' + styles + '" type="text/css">');
</script>
<style>
li{
    padding-left: 0px;
}
</style>
<link rel="stylesheet" href="../stylesheets/admin.1280.css" type="text/css">
</head>
<body class=menuBackground>

<table align=left border="0" cellspacing="0" cellpadding="1" style="margin-left:5px;margin-top:15px;">

    <tr><td style="padding-bottom:5px;padding-left:0px;text-align:left;"><b>Reports</b></td></tr>

    <tr><td nowrap style="padding-bottom:5px;">
        <li><a class="amenu" href="../reports.php" target="content">Summary</a>
    </td></tr>

<?php if(count($reportList) > 0): ?>

    <?php foreach($reportList as $file=>$label): ?>

    <tr><td nowrap style="padding-bottom:5px;">
        <li><a class="amenu" href="../reports/<?=$file;?>" target="content"><?=$label;?></a>
    </td></tr>

    <?php endforeach; ?>

<?php endif; ?>

</table>
</body>
</html>














