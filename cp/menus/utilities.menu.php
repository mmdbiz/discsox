<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "utilities";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

// Search reports directory for .php files and display as reports
$path = "../utilities";
$utilityList = array();
$handle = opendir($path);
while ($file = readdir($handle)) {
    if(is_file("$path/$file") && preg_match("/.php$/i", $file)) {
        $names = preg_split("/\./",$file);
        //lose the extension
        array_pop($names);
        $utilityName = join(" ", $names);
        $utilityList[$file] = ucwords($utilityName);
    }
}
ksort($utilityList);
?>

<html>
<head>
<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5" />
<title>Utilities Menu</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<script type="text/javascript">
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
    document.write('<link rel="stylesheet" href="../stylesheets/' + styles + '" type="text/css">');
//-->
</script>
<base target="content" />
</head>
<body class="menuBackground">
<table align=left border="0" cellspacing="0" cellpadding="1" style="margin-left:5px;margin-top:15px;">

    <tr><td style="padding-bottom:5px;padding-left:0px;text-align:left;"><b>Utilities</b></td></tr>

<?php if(count($utilityList) > 0): ?>

    <?php foreach($utilityList as $file=>$label): ?>

    <tr><td nowrap style="padding-bottom:5px;">
        <li><a class="amenu" href="../utilities/<?=$file;?>" target="content"><?=$label;?></a>
    </td></tr>

    <?php endforeach; ?>

<?php endif; ?>

</table>

</body>
</html>











