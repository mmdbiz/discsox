<?php
$_isAdmin = true;
$_adminFunction = "file.manager";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$fileManager = $_Registry->LoadClass("file.manager");
$root = $fileManager->rootDir;
$fileManager->lookInSubdirectories = true;
$dir = NULL;

$haveDir = false;
if(!empty($_REQUEST['dir'])){
	$dir = NULL;
	$dir = trim($_REQUEST['dir']);
	$haveDir = true;
}
$loadContent = true;
if(!empty($_REQUEST['noload'])){
	$loadContent = false;
}


$fileManager->listDirectories();
// put root dir in here...
array_unshift($fileManager->dirIndex,"");

?>


<html>
<head>
<meta http-equiv="cache-control" content="no-cache">
<meta http-equiv="pragma" content="no-cache">
<meta http-equiv="expires" content="0">
<title>File Manager</title>

<script language="JavaScript">
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
//-->
</script>
<style type="text/css">
body {
	margin-left: 0;
	margin-top: 10;
	margin-right: 0;
	margin-bottom: 0;
}
td{
	line-height: 17px;
}
.blankRow{
	line-height: 5px;
}
.selected{
	color:#FF0000;
}
</style>
</head>
<body class=menuBackground>
<div align="left">

	<table border="0" cellspacing="0" cellpadding="2" style="margin-top:10px;">
		<tr>
			<td colspan="2" style="padding-left:20px;"><b>Directory List</b></td>
		</tr>

	
	<?php foreach($fileManager->dirIndex as $i=>$key):?>
		<?php
			$folderIcon = "closed.folder.gif";
			if($dir == $key){
				$folderIcon = "open.folder.gif";
			}
		?>
		<tr>
			<td>
				<a href="file.menu.php?dir=<?=$key;?>"><img src='icons/<?=$folderIcon;?>' width='16' height='16' border="0"></a>
			</td>
			<td>
				<?php if($key == $dir):?>
					<?php if($dir == ""):?>
						<a id="<?=$key;?>" href="file.menu.php?dir=<?=$key;?>"><font color="red"><b>..</b></font></a>
					<?php else:?>
						<a id="<?=$key;?>" href="file.menu.php?dir=<?=$key;?>"><font color="red"><b><?=$key;?></b></font></a>
					<?php endif;?>
				<?php else:?>
					<?php if($key == ""):?>
						<a id="<?=$key;?>" href="file.menu.php?dir=<?=$key;?>">..</a>
					<?php else:?>
						<a id="<?=$key;?>" href="file.menu.php?dir=<?=$key;?>"><?=$key;?></a>
					<?php endif;?>
				<?php endif;?>
			</td>
		</tr>
	<?php endforeach;?>
	</table>

</div>
<?php if($loadContent):?>
<script language="JavaScript">
	parent.content.location = "file.manager.php?dir=<?=$dir;?>";
</script>
<?php endif;?>
</body>
</html>