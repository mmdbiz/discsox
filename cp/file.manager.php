<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "file.manager";

//print "<div align=left><pre>\n";
//print_r($_POST);
//print "</pre></div>\n";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$fileManager = $_Registry->LoadClass("file.manager");
$root = $fileManager->rootDir;
$fileManager->lookInSubdirectories = false;

$cwd = $root;
$requestDir = NULL;
$backOne = "";
$editText = false;
$editImage = false;
$uploadStatus = NULL;
$uploadFile = false;

// Edit a file
if(!empty($_REQUEST['edit'])){
	$editFile = $root . "/" . trim($_REQUEST['edit']);
	if(file_exists($editFile)){
		$info = pathinfo(basename($editFile));
		if(isset($info['extension']) && isset($fileManager->imageTypes[$info['extension']])){
			// it's an image
			$editImage = true;
			$imagePath = "../" . trim($_REQUEST['edit']);
		}
		else{
			$fileText = htmlentities(file_get_contents($editFile));
			$editText = true;
		}
	}
	else{
		die("Cannot find: $editFile");	
	}
}
$requestDir = null;
// Set selected directory
if(isset($_REQUEST['dir']) && trim($_REQUEST['dir']) != ""){
	$requestDir = trim($_REQUEST['dir']);
	if(!is_dir($root ."/". $requestDir)){
		die("Invalid Directory");
	}
	$parts = explode("/",$requestDir);
	array_pop($parts);
	$backOne = join("/",$parts);
	$cwd .= "/$requestDir";
}

// Update a file
if(isset($_REQUEST['update']) && !empty($_REQUEST['editFile'])){
	$file = $cwd ."/". $_REQUEST['filename'];
	$data = stripslashes($_REQUEST['editFile']);
	$fileManager->update($file,$data);
	
	$rFile = $_REQUEST['filename'];
	if($requestDir){
		$rFile = $requestDir ."/". $rFile;
	}
	$_Common->redirect("file.manager.php?dir=$requestDir&edit=$rFile");
	exit;
}

// Delete files
if(isset($_REQUEST['sel']) && count($_REQUEST['sel']) > 0){
	if(isset($_REQUEST['delete'])){
		//$_Common->debugPrint($_REQUEST['sel'],"Files to delete");
		$fileManager->delete();
	}
}

// Download files
if(!empty($_REQUEST['download'])){
	$filePath = $cwd . "/" . trim($_REQUEST['download']);
	$fileManager->do_download($filePath);
	exit;
}


// file uploads
if(isset($_REQUEST['doupload']) && isset($_FILES)){
	$fileManager->doUploads($cwd);
	$uploadFile = false;
}
elseif(isset($_REQUEST['showupload'])){
	$uploadFile = true;
}

// make a directory
if(isset($_REQUEST['makedir']) && !empty($_REQUEST['newdir'])){
	mkdir($cwd . "/" . $_REQUEST['newdir']);
}

// Create file index for display
if(!$editImage && !$editText){
	$fileManager->listDirectories($root ."/". $requestDir);
}

// row backround colors
$color = array();
$color[0] = "#FFFFFF";
$color[~0] = "#F8F8F8";
$ck = 0;

?>
<html>
<head>
<title>File Manager</title>
<script language="JavaScript">
//<!--
if(eval(parent.menu)) {
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "file.menu.php"){
		parent.menu.location = 'file.menu.php';
	}
}
function copyHtml() { 
	document.forms[0].elements['editFile'].select();
	document.forms[0].elements['editFile'].focus();  
	textRange = document.forms[0].elements['editFile'].createTextRange(); 
	textRange.execCommand("RemoveFormat"); 
	textRange.execCommand("Copy"); 
	alert("The text has been copied to your clipboard."); 
}
function highLightMenuLink(id){
	// highlights the menu entries
	for(i = 0;i < parent.menu.document.links.length;i++){
		if(parent.menu.document.links[i].id == id &&
		   parent.menu.document.links[i].innerHTML.toLowerCase() != '<font color=red><b>' + id + '</b></font>'){
			parent.menu.location = 'file.menu.php?noload=1&dir=' + id;
			break;
		}
	}
}

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
    
</script>
<style>

.fileRow{
	vertical-align: middle;
	font-size: 10px;
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 0px 0px 1px 1px;
}
.fileRowEnd{
	vertical-align: middle;
	font-size: 10px;
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 0px 1px 1px 1px;
}

.footerRow{
	vertical-align: middle;
	font-size: 10px;
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 0px 1px 1px 1px;
}


.fileHeader{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 0px 1px 1px;
	background-color: #E5E5E5;
	color: #000000;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
.fileHeaderEnd{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 1px 1px 1px;
	background-color: #E5E5E5;
	color: #000000;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}

.uploadHeader{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 0px 1px 1px;
	background-color: #F5F5F5;
	color: #000000;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
.uploadHeaderEnd{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 1px 1px 1px;
	background-color: #F5F5F5;
	color: #000000;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}





.titleHeader{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 0px 1px 1px;
	background-color: #E5E5E5;
	color: #000000;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
.titleHeaderMid{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 0px 1px 0px;
	background-color: #E5E5E5;
	color: #000000;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
.titleHeaderEnd{
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 1px 1px 0px;
	background-color: #E5E5E5;
	color: #000000;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
.allBorder{
	vertical-align: middle;
	font-size: 11px;
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 1px 0px 1px;
}
input{
     font-size: 12px;
     font-family: Courier New,sans-serif;
}


</style>
</head>
<body>
<div align="center">
<form name="filelist" action="file.manager.php" method="post" enctype="multipart/form-data">
<input type="hidden" name="dir" value="<?=$requestDir;?>">

<?php if($editText || $editImage):?>

	<?php 
		$fSize = $fileManager->convertFileSize(filesize($editFile));
		if(!empty($_SERVER['PHP_SELF'])){
			
			$self = $_SERVER['PHP_SELF'];
			$self = str_replace('//','/',$self);
			
			$refer = "$self?dir=$requestDir";
		}
		else{
			$refer = "javascript:history.back();";	
		}
	?>

	<table border="0" cellspacing="0" cellpadding="0" width="100%" 
		   style="border-style:solid;border-color:#CDCDCD;border-width:1px 1px 1px 1px;">
		<tr bgcolor="#F8F8F8" style="padding=left:5px;padding-right:5px;line-height:10px;">
			<td style="vertical-align:middle;" nowrap>
				<a href="<?=$refer;?>" class="back" onMouseOut="this.className='back';" onMouseOver="this.className='backhover';" target="content">
				<img src="icons/back.arrow.gif" border="0" height="20" width="20" style="vertical-align:middle;">
				<b>BACK</b></a>
			</td>
			<td width="80%" align="center">
				<b>File:</b> <?=$editFile;?> (<?=$fSize;?>)
			</td>
			<td align="right" nowrap>
				<?php if(!$editImage):?>
				<span class="back"
						onMouseOut="this.className='back';"
						onMouseOver="this.className='backhover';"
						onClick="javascript:copyHtml();">
				<b>Copy text to clipboard</b></a>
				<?php else:?>
					&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;
				<?php endif;?>
			</td>
		</tr>
	</table>

	<?php if($editImage):?>

		<?php
			$iSize = getImageSize($editFile);
		?>

		<table border="0" cellspacing="0" cellpadding="3" width="100%">
			<tr><td colspan="2" style="line-height:15px;">&nbsp;</td></tr>
			<tr>
				<td align="right" valign="top">Image Name: </td>
				<td valign="top" width="80%"><?=basename($imagePath);?></td>
			</tr>
			<tr>
				<td align="right" valign="top">Size: </td>
				<td valign="top" width="80%"><?=$fSize;?></td>
			</tr>
			<tr>
				<td align="right" valign="top">Height: </td>
				<td valign="top" width="80%"><?=$iSize[1];?></td>
			</tr>
			<tr>
				<td align="right" valign="top">Width: </td>
				<td valign="top" width="80%"><?=$iSize[0];?></td>
			</tr>
			
			<?php if(substr(basename($imagePath),-3) == "swf"):?>
			<tr>
				<td align="right" valign="top">Preview: </td>
				<td valign="top" width="80%">
					<object codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,29,0"
					height="<?=$iSize[1];?>" width="<?=$iSize[0];?>" classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000">
					<param name="movie" value="<?=$imagePath;?>">
					<param name="quality" value="high">
					<embed src="<?=$imagePath;?>" quality="high" pluginspage="http://www.macromedia.com/go/getflashplayer"
						   type="application/x-shockwave-flash" width="<?=$iSize[0];?>" height="<?=$iSize[1];?>"></embed>
					</object>
				</td>
			</tr>
			<?php else:?>
			<tr>
				<td align="right" valign="top">Preview: </td>
				<td valign="top" width="80%">
					<img src="<?=$imagePath;?>" width="<?=$iSize[0];?>" height="<?=$iSize[1];?>">
				</td>
			</tr>
			<?php endif;?>
			
			
		</table>


	<?php else:?>
	
		<input type="hidden" name="filename" value="<?=basename($editFile);?>">
		<table border="0" cellspacing="0" cellpadding="0" width="100%">
			<tr><td style="line-height:5px;">&nbsp;</td></tr>
			<tr>
				<td>
					<textarea name="editFile" style="font-family:Courier New;font-size:12px;width:100%;" rows="27" wrap="off"><?=$fileText;?></textarea>
				</td>
			</tr>
			<tr><td style="line-height:15px;">&nbsp;</td></tr>
			<tr>
				<td align="center">
					<input type="submit" name="update" value="Update File">
				</td>
			</tr>
		</table>

	<?php endif;?>

<?php elseif($uploadStatus):?>

	<h4>Upload Status</h4>
	<table border="0" cellspacing="0" cellpadding="5" width="70%">
		<tr>
			<th class="fileHeader" align="left">Filename</th>
			<th class="fileHeaderEnd" width="70%" align="left">Result</th>
		</tr>

		<?php foreach($uploadStatus as $name=>$result):?>
			<tr bgcolor="<?=$color[$ck = ~$ck];?>">
				<td class="fileRow"><?=$name;?></td>
				<td class="fileRowEnd"><?=$result;?></td>
			</tr>
		<?php endforeach;?>
	</table>
	<p><a href="file.manager.php?dir=<?=$requestDir;?>">Return to file list</a></p>

<?php elseif($uploadFile):?>
	
	<input type="hidden" name="showupload" value="1">
	
	<table border="0" cellspacing="0" cellpadding="5" width="70%">
		<tr bgcolor="<?=$color[$ck = ~$ck];?>">
			<td class="uploadHeader" align="right"><b>Current Directory:</b></td>
			<td class="uploadHeaderEnd"><?=$cwd;?></td>
		</tr>
		<tr bgcolor="<?=$color[$ck = ~$ck];?>">
			<td align="right" class="fileRow"><b>Select File Count:</b></td>
			<td class="fileRowEnd" width="80%">
				<?php
					$fCount = 1;
					if(!empty($_REQUEST['filecount'])){
						$fCount = $_REQUEST['filecount'];
					}
					list($fselect,$junk) = $_Common->makeSelectBox("filecount",array(1,5,10),array(1,5,10),$fCount,true);
				?>
				<?=$fselect;?>
			</td>
		</tr>
		
		<?php for($i=1;$i<=$fCount;$i++):?>
			<tr bgcolor="<?=$color[$ck = ~$ck];?>">
				<td class="fileRow" align="right">File <?=$i;?>:</td>
				<td class="fileRowEnd"><input name="upfile[]" type="file" size="40"></td>
			</tr>
		<?php endfor;?>

		<tr bgcolor="<?=$color[$ck = ~$ck];?>">
			<td colspan="2" align="center" class="footerRow">
				<input type="submit" name="doupload" value="Submit">
			</td>
		</tr>
	</table>
	
<?php else:?>
	
	<table border="0" cellspacing="0" cellpadding="5" width="700" style="table-layout:fixed">
		<col width=50>
		<col width=250>
		<col width=100>
		<col width=100>
		<col width=100>
		<col width=100>
		
		<tr bgcolor="<?=$color[$ck = ~$ck];?>">
			<td class="allBorder" colspan=6><b>Current Directory:</b> <?=$cwd;?></td>
		</tr>
		<tr>
			<th class="fileHeader">&nbsp;</th>
			<th class="fileHeader" align="left">Name</th>
			<th class="fileHeader" align="center" nowrap>Size</th>
			<th class="fileHeader" align="center">Permissions</th>
			<th class="fileHeader" align="center">Select</th>
			<th class="fileHeaderEnd" align="center">Download</th>
		</tr>
		<tr>
			<td class="fileRow" align="center">
				<a href="file.manager.php?dir=<?=$backOne;?>" title="Go back one directory" onClick="highLightMenuLink('<?=$backOne;?>');">
				<img src='icons/back.arrow.gif' width='20' height='20' border="0"></a>
			</td>
			<td class="fileRow">
				<a href="file.manager.php?dir=<?=$backOne;?>" title="Go back one directory" onClick="highLightMenuLink('<?=$backOne;?>');"><b>..</b></a>
			</td>
			<td class="fileRow">&nbsp;</td>
			<td class="fileRow">&nbsp;</td>
			<td class="fileRow">&nbsp;</td>
			<td class="fileRowEnd">&nbsp;</td>
		</tr>

		<?php foreach($fileManager->dirIndex as $i=>$subdir):?>
		
		<?php $dPerms = ""; $dirPath = "";?>
		
		<tr bgcolor="<?=$color[$ck = ~$ck];?>">
			<td class="fileRow" align="center"><img src='icons/closed.folder.gif' width='16' height='16' border="0"></td>
			<?php if($requestDir):?>
				<?php
					$dirPath = $root."/".$requestDir."/".$subdir;
					$dPerms = $fileManager->getFilePermissions(fileperms($dirPath));
					$dChmod = $fileManager->chmodnum($dPerms);
				?>
				<td class="fileRow"><a href="file.manager.php?dir=<?=$requestDir."/".$subdir;?>" title="Open <?=$subdir;?> Directory" onClick="highLightMenuLink('<?=$requestDir;?>');"><?=$subdir;?></a></td>
			<?php else:?>
				<?php
					$dirPath = $root."/".$subdir;
					$dPerms = $fileManager->getFilePermissions(fileperms($dirPath));
					$dChmod = $fileManager->chmodnum($dPerms);
				?>
				<td class="fileRow"><a href="file.manager.php?dir=<?=$subdir;?>" title="Open <?=$subdir;?> Directory" onClick="highLightMenuLink('<?=$subdir;?>');"><?=$subdir;?></a></td>
			<?php endif;?>
			<td class="fileRow" align="center">&nbsp;</td>
			<td class="fileRow" align="center"><?=$dPerms;?><br/>(<?=$dChmod;?>)</td>
			<td class="fileRow" align="center"><input type="checkbox" name="sel[]" value="<?=$dirPath;?>"></td>
			<td class="fileRowEnd">&nbsp;</td>
		</tr>
		<?php endforeach;?>


		<?php foreach($fileManager->fileIndex as $file=>$flds):?>
			
			<?php
				$fileToEdit = $file;
				if($requestDir){
					$fileToEdit = "$requestDir/$file";
				}
				$flds['chmod'] = $fileManager->chmodnum($flds['perms']);
			?>
		
			<tr bgcolor="<?=$color[$ck = ~$ck];?>">
				<td class="fileRow" align="center"><?=$flds['icon'];?></td>
				<td class="fileRow" align="left"><a href="file.manager.php?edit=<?=$fileToEdit;?>&dir=<?=$requestDir;?>" title="View/Edit <?=$file;?>"><?=$file;?></a></td>
				<td class="fileRow" align="center" nowrap><?=$flds['size'];?></td>
				<td class="fileRow" align="center"><?=$flds['perms'];?> (<?=$flds['chmod'];?>)</td>
				<td class="fileRow" align="center"><input type="checkbox" name="sel[]" value="<?="$cwd/$file";?>"></td>
				<td class="fileRowEnd" align="center"><a href="file.manager.php?download=<?=$file;?>&dir=<?=$requestDir;?>">Download</a></td>
			</tr>
		<?php endforeach;?>
		
		<tr><td colspan="6">&nbsp;</td></tr>
		<tr>
			<td colspan="6" align="right">
			<table border="0" cellspacing="0" cellpadding="2">
				<tr>
					<td align="right">&nbsp;</td>
					<td align="right">
						<input type="submit" name="delete" value="Delete Selections" onClick="return confirm('Are you sure you want to delete these selections?\n\nIf you selected a directory, this will also delete all files and subdirectories within the selected directory.');">
					</td>
				</tr>
				<tr>
					<td align="right">&nbsp;</td>
					<td align="right"><input type="button" value="Upload New Files" onClick="javascript:document.location='file.manager.php?showupload=1&dir=<?=$requestDir;?>';"></td>
				</tr>
				<tr>
					<td align="right">Create Sub-directory:	</td>
					<td>
						<input name="newdir" type="text" size="24">
						<input type="submit" name="makedir" value="Go">
					</td>
				</tr>
			</table>
			</td>
		</tr>
	</table>

<?php endif;?>

<p>&nbsp;</p>
</form>
</div>
</body>
</html>
	
	
	