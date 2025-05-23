<?php
//VersionInfo:Version[3.0.1] 
$_isAdmin = true;
$_adminFunction = "utilities";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(false,true);

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

include_once("../include/updates.inc");
$updates = new Updates();

$step = null;
$showFileTypes = false;

if(isset($_REQUEST['getfiles']) && !empty($_REQUEST['files']) && !empty($_REQUEST['email_address'])){
	$updates->downloadFiles();
	$step = 1;
}
elseif(isset($_REQUEST['unzipfiles'])){
	$updates->unzipFiles();
	$updates->checkGateways();
	$step = 2;
}
elseif(isset($_REQUEST['checkupdates'])){
	$step = 0;
	$updates->checkVersions();
}
else{
	$showFileTypes = true;
}

// row backround colors
$color = array();
$color[0] = "#FFFFFF";
$color[~0] = "#E2EDE2";
$ck = 0;

?>
<html>
<head>
<title>Check for Updates</title>
<style>
.borderBox{
	vertical-align: top;
	font-size: 10px;
	border-style: solid;
	border-color: #CDCDCD;
	border-width: 1px 1px 1px 1px;
}
</style>
<script LANGUAGE="JavaScript">
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
document.write('<link rel="stylesheet" href="../stylesheets/' + styles + '" type="text/css">');

function selectAll(form, select){
    for(var i=0;i < form.length;i++){
        fldObj = form.elements[i];
        if(fldObj.type == 'checkbox'){
            if(select){
                fldObj.checked = true;
            }
            else{
                fldObj.checked = (fldObj.checked) ? false : true;
            }
        }
    }
}

function checkEmail(form){
	if(!emailCheck(form.email_address.value)){
		form.email_address.select();
		alert("Please enter a valid email address");
		return false;
	}
	else{
		var isChecked = false;
		for(var i=0;i < form.length;i++){
			fldObj = form.elements[i];
			if(fldObj.type == 'checkbox' && fldObj.checked){
				isChecked = true;
				break;
			}
		}
		if(!isChecked){
			alert("You have not checked a file to download?");
			return false;
		}
	}
	return true;
}

function emailCheck(str){
    var at="@"
    var dot="."
    var lat=str.indexOf(at);
    var lstr=str.length;
    var ldot=str.indexOf(dot);
    if (str.indexOf(at)==-1){
       return false;
    }
    if (str.indexOf(at)==-1 || str.indexOf(at)==0 || str.indexOf(at)==lstr){
       return false;
    }
    if (str.indexOf(dot)==-1 || str.indexOf(dot)==0 || str.indexOf(dot)==lstr){
        return false;
    }
    if (str.indexOf(at,(lat+1))!=-1){
       return false;
    }
    if (str.substring(lat-1,lat)==dot || str.substring(lat+1,lat+2)==dot){
       return false;
    }
    if (str.indexOf(dot,(lat+2))==-1){
       return false;
    }
    if (str.indexOf(" ")!=-1){
       return false;
    }
    return true;
}
</script>
</head>
<body class="mainBody">

	<div align="center">

		<?php if($updates->error):?>
		
			<p><b><?=$updates->error;?></b></p>

		<?php elseif($showFileTypes):?>

			<form name="update" id="update" action="check.for.updates.php" method="post">
				<table border="0" cellpadding="3" cellspacing="0" width="600">
					<tr>
						<th colspan="2" align="left" height="17">Check For Program Updates</th>
					</tr>
					<tr>
						<td colspan="2">&nbsp;</td>
					</tr>
					<tr>
						<td align="right" valign="top" style="padding-top:8px;">Select File Types to Check:</td>
						<td valign="middle">
						
							<?php foreach($updates->validExtensions as $i=>$type):?>
								<input type="checkbox" name="extensions[]" value="<?=$type;?>" checked> .<?=$type;?>&nbsp;
							<?php endforeach;?>
						
						</td>
					</tr>
					<tr>
						<td colspan="2">&nbsp;</td>
					</tr>
					<tr>
						<td colspan="2" align="center">
							<input type="submit" id="checkupdates" name="checkupdates" value="Check For Updates">
						</td>
					</tr>
				</table>
			</form>

		<?php elseif($updates->success):?>

			<?php if($step == 1):?>
			
				<p><?=$updates->updateFilename;?> has been downloaded.</p>
				<p><a href="check.for.updates.php?unzipfiles=1">Click Here</a> to install these files.</p>

			<?php elseif($step == 2):?>
			
				<?php if(count($updates->updatedFiles) > 0):?>
					<p>The following files have been updated:</p>
					<table border="1" cellpadding="3" cellspacing="0" width="550">
						<?php foreach($updates->updatedFiles as $i=>$fname):?>
							<tr bgcolor="<?=$color[$ck = ~$ck];?>">
								<td width="550">
									<?=$fname;?>
								</td>
							</tr>
						<?php endforeach;?>
					</table>
				<?php endif;?>

				<?php if(count($updates->newGateways) > 0):?>
					<p>The following payment gateway files have been installed:</p>
					<table border="1" cellpadding="3" cellspacing="0" width="550">
						<?php foreach($updates->newGateways as $i=>$fname):?>
							<tr bgcolor="<?=$color[$ck = ~$ck];?>">
								<td width="550">
									<?=$fname;?>
								</td>
							</tr>
						<?php endforeach;?>
					</table>
				<?php endif;?>
				
			<?php endif;?>

		<?php else:?>

			<?php if(count($updates->newFiles) > 0 || count($updates->filesToUpdate) > 0):?>

				<form name="update" id="update" action="check.for.updates.php" method="post">

					<table border="0" cellpadding="1" cellspacing="0" width="600" class="borderBox">
						<tr>
							<td colspan="2">&nbsp;</td>
						</tr>
						<tr>
							<td colspan="2" align="center">
							<b>Select the files you want updated.<br /><br /><font color="red">Do not select any files you have customized. This will overwrite them!</font></b>
							</td>
						</tr>
						<tr>
							<td colspan="2">&nbsp;</td>
						</tr>
						<?php if(count($updates->newFiles) > 0):?>

							<tr>
								<th colspan="2" align="left" height="17">New files added to the build</th>
							</tr>
							<?php foreach($updates->newFiles as $fname=>$md5):?>
								<tr bgcolor="<?=$color[$ck = ~$ck];?>">
									<td align="center" width="50">
										<input type="checkbox" name="files[]" value="<?=$fname;?>">
									</td>
									<td width="550">
										<?=$fname;?>
									</td>
								</tr>
							<?php endforeach;?>
							<?php if(count($updates->filesToUpdate) > 0):?>
								<tr>
									<td colspan="2">&nbsp;</td>
								</tr>
							<?php endif;?>
						
						<?php endif;?>

						<?php if(count($updates->filesToUpdate) > 0):?>
							<tr>
								<th colspan="2" align="left" height="17">Files available for update</th>
							</tr>
							<?php foreach($updates->filesToUpdate as $fname=>$md5):?>
								<tr bgcolor="<?=$color[$ck = ~$ck];?>">
									<td align="center" width="50">
										<input type="checkbox" name="files[]" value="<?=$fname;?>">
									</td>
									<td width="550">
										<?=$fname;?>
									</td>
								</tr>
							<?php endforeach;?>
						<?php endif;?>
						<tr>
							<td colspan="2"><hr size="1" noshade></td>
						</tr>
						<tr>
							<td colspan="2" style="padding-top:15px;padding-bottom:5px;" align="center">

							<p><a href="javascript:selectAll(document.forms['update'],true);">Select All</a>&nbsp;-&nbsp;
								<a href="javascript:selectAll(document.forms['update'],false);">Unselect All</a><br /><br /></p>
								
								<table border="0" cellpadding="3" cellspacing="0" width="100%">
									<tr>
										<td colspan="2" align="center">
											To download the selected files, enter your customer email address below:
										</td>
									</tr>
									<tr>
										<td colspan="2">&nbsp;</td>
									</tr>
									<tr>
										<td align="right">Email Address: </td>
										<td><input type="text" name="email_address" size="25"></td>
									</tr>
									<tr>
										<td colspan="2">&nbsp;</td>
									</tr>
									<tr>
										<td colspan="2" align="center">
											Should we use Passive mode when uploading the files? Some servers require this.
										</td>
									</tr>
									<tr>
										<td align="right">Use FTP Passive Mode:</td>
										<td><input type="checkbox" name="passive" value="true" checked></td>
									</tr>
									<tr>
										<td colspan="2">&nbsp;</td>
									</tr>
									<tr>
										<td colspan="2" align="center">
											<input type="submit" id="getfiles" name="getfiles" value="Get Files" onclick="return checkEmail(this.form);">	
										</td>
									</tr>
								</table>

							</td>
						</tr>
					</table>
				</form>

			<?php else:?>

				<p><br />There are no new files -or- files that need to be updated.</p>

			<?php endif;?>


		<?php endif;?>
		
	</div>
<p>&nbsp;</p>
<p>&nbsp;</p>
</body>
</html>

