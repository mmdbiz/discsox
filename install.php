<?php
session_start();
error_reporting(E_ERROR|E_PARSE);

include_once("include/common.inc");
$_Common = new Common();
$_CF = array();

$ok = true;
$dbError = false;
$connectError = false;

$step = intval(1);
if(!empty($_GET['step']) && is_numeric($_GET['step'])){
	$step = intval($_GET['step']);
}

$cwd = getcwd();

$ftpcwd = null;

if(trim($cwd) == ""){
	$ok = false;
	$dbError = "The install program was unable to read the current working directory from your server.<br /><br />";
	$dbError .= "You will have to set the \$cwd variable in the install.php on line 14 manually to continue";
}
else{
	$cwd = str_replace("\\","/",$cwd);
	$dir = $cwd;
	$cpanelURL = "cp";
}

// --------------------------------------------------------------
function doDatabaseInstall(){

	global $dbError;
	global $connectError;
	global $dir;

	if(empty($_GET['db_host']) || empty($_GET['db_username']) || empty($_GET['db_name'])){
		$dbError = "Missing Database Parameters.\n";
		return false;
	}
	
    $hostname = trim(stripslashes($_GET['db_host']));
    $username = trim(stripslashes($_GET['db_username']));
    $password = trim(stripslashes($_GET['db_password']));
    $databasename = trim(stripslashes($_GET['db_name']));

    $conn = @mysql_connect($hostname, $username, $password);
    if(!$conn){
        $dbError = "Connection to mySql server failed\n" . mysql_error();
        $connectError = true;
        return false;
    }

	$dbrs = mysql_query('SHOW DATABASES;', $conn);
	$dbs = array();
	while($row = mysql_fetch_array($dbrs,MYSQL_ASSOC)){
		$dbs[$row['Database']] = 1;
	}

    // check to see if database (with tables) exists already
    if(isset($dbs[$databasename])){
		$trs = @mysql_query("SHOW TABLES IN `$databasename`", $conn);
		$tbls = array();
		$tblfield = "Tables_in_" . $databasename;
		
		while($row = @mysql_fetch_array($trs,MYSQL_ASSOC)){
			$tbls[$row[$tblfield]] = 1;
		}
		if(count($tbls) > 0 && $_GET['delete_tables'] == "yes"){
			$db = @mysql_select_db($databasename, $conn);
			if(!$db){
				$dbError = "Could Not select Database: $databasename\n" . mysql_error();
				return false;
			}
			else{
				$tblStr = "`" . join("`,`",array_keys($tbls)) . "`";
				if(!@mysql_query("DROP TABLE $tblStr", $conn)) {
					$dbError = "Error deleting table in database:\n" . mysql_error();
					return false;
				}
			}
		}
		elseif(count($tbls) > 0 && $_GET['add_tables'] == "no"){
			$dbError = "MySQL database &quot;$databasename&quot; already exists and has existing tables in it. 
						The database must be either be emptied or you can backup in your browser and use a different database name
						or you can select to remove the tables to continue.<br /><br />
						We will not overwrite an existing database during this installation.";
			return false;
		}
	}

	$createSQL = "CREATE DATABASE IF NOT EXISTS `$databasename`";

    if(!@mysql_query($createSQL, $conn)) {
		$dbError = "Error creating database:\n" . mysql_error();
		return false;
    }     
    
	$result = @mysql_select_db($databasename, $conn);
	if(!$result){
		$dbError = "Could Not Connect To Database: $databasename\n" . mysql_error();
		return false;
	}
	
	if(file_exists("$dir/cp/database/qs30.sql") && file_exists("$dir/cp/database/qs30.install.data.sql")){
		@chmod("$dir/cp/database/qs30.sql",0755);
		@chmod("$dir/cp/database/qs30.install.data.sql",0755);
		run_query_batch($conn, "$dir/cp/database/qs30.sql");
		run_query_batch($conn, "$dir/cp/database/qs30.install.data.sql"); 
	}
    else{
		$dbError = "Database SQL file missing in $dir/cp/database. Installation aborted...";
		return false;
	}

	// enter install date
	$sql = "INSERT INTO install (install_date,file_name) VALUES(CURDATE(),'default install')";
	$rs = mysql_query($sql, $conn);

	$_SESSION['db_host'] = $hostname;
	$_SESSION['db_username'] = $username;
	$_SESSION['db_password'] = $password;
	$_SESSION['db_name'] = $databasename;
	
	mysql_close($conn);
	
	//return writeToStartupFile();
	return true;
}

// ------------------------------------------------------------------
function run_query_batch($handle, $filename = "") 
{ 
  // -------------- 
  // Open SQL file. 
  // -------------- 
  if (!($fd = fopen($filename, "r")) ) { 
    die("Failed to open $filename: " . mysql_error() . "<br>"); 
  } 

  // -------------------------------------- 
  // Iterate through each line in the file. 
  // --------------------------------------
  $stmt = "";
  while(!feof($fd)) { 

    // ------------------------- 
    // Read next line from file. 
    // ------------------------- 
    $line = fgets($fd, 32768); 
    $stmt = "$stmt$line"; 

    // ------------------------------------------------------------------- 
    // Semicolon indicates end of statement, keep adding to the statement. 
    // until one is reached. 
    // ------------------------------------------------------------------- 
    if (!preg_match("/;/", $stmt)) { 
      continue; 
    } 

    // ---------------------------------------------- 
    // Remove semicolon and execute entire statement. 
    // ---------------------------------------------- 
    $stmt = preg_replace("/;/", "", $stmt); 

    // ---------------------- 
    // Execute the statement. 
    // ---------------------- 
    mysql_query($stmt, $handle) || 
		die("<pre>Query failed: " . mysql_error() . "\n$filename\n$stmt"); 
    $stmt = ""; 
  } 

  // --------------- 
  // Close SQL file. 
  // --------------- 
  fclose($fd); 
} 

// ------------------------------------------------------------------
function writeToStartupFile(){

	global $_CF;

	$hostname = $_SESSION['db_host'];
	$username = $_SESSION['db_username'];
	$password = $_SESSION['db_password'];
	$dbname = $_SESSION['db_name'];
	
	$data = "<?php\n";
	$data .= "// general database connection parameters\n";
	$data .= "define('DB_HOST', '$hostname');\n";
	$data .= "define('DB_USERNAME', '$username');\n";
	$data .= "define('DB_PASSWORD', '$password');\n";
	$data .= "define('DB_NAME', '$dbname');\n";
	$data .= "// session timeout in minutes\n";
	$data .= "define('SESSION_TIMEOUT', '60');\n";
	$data .= "// time offset in hours (minus signs are valid. Example: -1)\n";
	$data .= "define('TIME_OFFSET','0');\n";
	$data .= "?>";

	$_CF['ftp']['ftp_host'] = $_SESSION['ftp_host'];
	$_CF['ftp']['username'] = $_SESSION['ftp_username'];
	$_CF['ftp']['password'] = $_SESSION['ftp_password'];
	$_CF['ftp']['document_root'] = $_SESSION['ftp_document_root'];

	include_once("cp/include/ftp.inc");
	$ftp = new Ftp();
	if(isset($_SESSION['ftp_passive'])){
		$ftp->setPassiveMode(true);
	}

	if(!$ftp->ChDir($_CF['ftp']['document_root']."/include")){
		if(substr($ftp->error,0,30) == "Could not change ftp directory"){
			$dbError = "The FTP document root setting appears to be entered incorrectly. We were unable to write the startup.inc.php file.";
		}
		else{
			$dbError = $ftp->error;
		}
		return false;
	}

	$ftp->writeFile("startup.inc.php",$data);

	return true;
}


// ------------------------------------------------------------------
function setBasicParams(){

	global $cpanelURL;
	global $dbError;
	
    $conn = @mysql_connect($_SESSION['db_host'],$_SESSION['db_username'],$_SESSION['db_password']);
    if(!$conn){
        $dbError = "Connection to mySql server failed\n" . mysql_error();
        $connectError = true;
        return false;
    }
	$result = @mysql_select_db($_SESSION['db_name'], $conn);
	if(!$result){
		$dbError = "Could Not Connect To Database: $databasename\n" . mysql_error();
		return false;
	}

	foreach($_GET as $key=>$value){
		$_GET[$key] = trim(stripslashes($value));
	}
	
	$cpanelURL = $_GET['web_site_url'] . "/cp";
	
	$fields = array("store_name" => $_GET['store_name'] . ",basics",
					"web_site_url" => $_GET['web_site_url'] . ",basics",
					"secure_url" => $_GET['secure_url'] . ",basics",
					"store_email_address" => $_GET['store_email_address'] . ",email",
					"mail_server_host" => $_GET['mail_server_host'] . ",email",
					"mail_server_username" => $_GET['mail_server_username'] . ",email",
					"mail_server_password" => $_GET['mail_server_password'] . ",email",
					"security_phrase" => $_GET['security_phrase'] . ",encryption",
					"ftp_host" => $_SESSION['ftp_host'] . ",ftp",
					"username" => $_SESSION['ftp_username'] . ",ftp",
					"password" => $_SESSION['ftp_password'] . ",ftp",
					"document_root" => $_SESSION['ftp_document_root'] . ",ftp");
	

	foreach($fields as $name=>$value){
		list($val,$section) = explode(",",$value);
		$val = preg_replace("|\'|","`",$val);
		$sql = "UPDATE config SET `value` = '$val' WHERE `key` = '$name' AND `section` = '$section'";
		$rs = @mysql_query($sql, $conn);
		if(!$rs){
			$dbError = "Configuration update query failed: " . mysql_error() . "<br>"; 
			return false;
		}
	}

	if(empty($_GET['admin_password']) || empty($_GET['admin_email'])){
		$dbError = "Admin password or email address cannot be left blank";
		return false;
	}
	
	$adminPass = md5($_GET['admin_password']);
	$adminEmail = $_GET['admin_email'];
	$sql = "UPDATE users SET password = '$adminPass',email_address = '$adminEmail' WHERE username = 'ADMIN'";
	$rs = @mysql_query($sql, $conn);
	if(!$rs){
		$dbError = "Configuration update query failed: " . mysql_error() . "<br>"; 
		return false;
	}
	
	mysql_close($conn);
	return true;
}
// ------------------------------------------------------------------
function testFtp(){

	global $_CF,$_Common,$dbError;
	global $docRootList,$ftpcwd;

	if(!empty($_REQUEST['move'])){
		$_SESSION['ftp_ok'] = false;
	}

	if(empty($_REQUEST['move']) && !empty($_GET['ftp_host'])){
		$_SESSION['ftp_host'] = trim(stripslashes($_GET['ftp_host']));;
		$_SESSION['ftp_username'] = trim(stripslashes($_GET['ftp_username']));
		$_SESSION['ftp_password'] = trim(stripslashes($_GET['ftp_password']));
	}

	if(!$ftpcnx = ftp_connect($_SESSION['ftp_host'],21,10)){
		$dbError = "Could not connect to FTP Server";
		return false;
	}
	if(!$login = ftp_login($ftpcnx, $_SESSION['ftp_username'], $_SESSION['ftp_password'])){
		$dbError = "Could not login to FTP Server. Connection refused.";
		return false;
	}
	
	if(isset($_GET['ftp_passive']) || isset($_SESSION['ftp_passive'])){
		ftp_pasv($ftpcnx, true);
		$_SESSION['ftp_passive'] = true;
		$_Common->debugPrint("USing Passive Mode");
	}

	$docRootList = array();

	if(empty($_SESSION['docRootList']) || !empty($_REQUEST['move'])){
		
		if(!empty($_REQUEST['move'])){
			@ftp_chdir($ftpcnx, $_REQUEST['move']);
		}
		
		$path = ftp_pwd($ftpcnx);
		$ftpcwd = $path;

//$_Common->debugPrint($ftpcwd);
		
		$docRootList[] = $path;
		getDirList($ftpcnx,$path,$docRootList);
		
//$_Common->debugPrint($docRootList);
		
		usort($docRootList,'strcasecmp');

//$_Common->debugPrint($docRootList);
		
		if(count($docRootList) > 0){
			$_SESSION['docRootList'] = $docRootList;
		}
		else{
			$_SESSION['docRootList'] = null;	
		}
	}
	else{
		$docRootList = $_SESSION['docRootList'];
	}

	return true;	
}

// ----------------------------------------------------------------
function getDirList($ftpcnx,$path,&$dirs){

	global $_Common,$ftpcwd;

	$res = ftp_rawlist($ftpcnx,$path,false);

	//$_Common->debugPrint($res,"Results");

	if(count($res) > 0){
		
		foreach($res as $i=>$dir){
			
			if(strstr($dir,'cgi-bin')){
				continue;	
			}
			elseif(substr($dir,0,1) == 'l'){
				
				$parts = preg_split("/[\s]+/",$dir);
				
				$simDir = $parts[count($parts) - 3];
				
				if(strstr($simDir,'../')){
					$simDir = "/" . str_replace('../','',$simDir);
				}
				
				$simDir = str_replace('//','/',$simDir);
				
				// we don't want files
				if(!strstr($simDir,'.')){
					$dirs[] = $simDir;
				}
			}
			elseif(substr($dir,0,1) == 'd' && (!strstr($dir,'mainwebsite_cgi') && !strstr($dir,'mainwebsite_html'))){
				$parts = preg_split("/[\s]+/",$dir);
				$dir = trim(array_pop($parts));
				if($dir == '.' || $dir == '..' || substr($dir,0,1) == '_'){
					continue;	
				}
				$dirs[] = trim($dir);	
			}
		}
	}
}

?>

<?php
	if($ok){
		switch ($step) {
			case 2:
				if(empty($_SESSION['db_host'])){
					$ok = doDatabaseInstall();
				}
				break;
			case 3:
				if(!empty($_GET['ftp_host']) || !empty($_SESSION['ftp_host'])){
					$ok = testFtp();
				}
				else{
					$dbError = "FTP host missing. The FTP information is required to continue.";
					$ok = false;
				}
				break;
			case 4:
				if(!empty($_GET['ftp_document_root'])){
					$_SESSION['ftp_document_root'] = trim(stripslashes($_GET['ftp_document_root']));
					$ok = writeToStartupFile();	
				}
				else{
					$dbError = "FTP document root missing. This information is required to continue.";
					$ok = false;
				}
				break;
			case 5:
				if(!empty($_SESSION['db_host'])){
					$ok = setBasicParams();
				}
				else{
					$dbError = "Database host missing. You will need to start at step one. Please close your browser and start again...";
					$ok = false;
				}
				break;
			default:
				// show first screen
				break;
		}
	}
?> 

<html>
<head>
<title>Installing QuikStore 3.0 - Step <?=$step;?></title>
<style>
h4 {
     font-family: Verdana, Arial, Helvetica, sans-serif;
     color: #0005CE;
     font-size: 14px;
}
p {
     font-family: Verdana, Arial, Helvetica, sans-serif;
     color: #000000;
     font-size: 11px;
}
td {
     font-family: Verdana, Arial, Helvetica, sans-serif;
     color: #000000;
     font-size: 11px;
}
th {
     font-family: Verdana, Arial, Helvetica, sans-serif;
     color: #000000;
     font-size: 11px;
     line-height: 11px;
     background-color: #CDCDCD;
}
select, option {width:400px;overflow:auto;}

</style>
</head>
<body>
<div align="center">
<img src="images/disc_box_sm.png" width="121" height="100" border="0" align="middle"><img src="images/q30.gif" width="219" height="30" border="0"><br>
<form method="GET" action="install.php">

<?php if($dbError || !$ok):?>

	<h4><br />&nbsp;</h4>
	<table border="0" cellpadding="2" width="600">
		<tr>
			<td>
				<?php if($connectError):?>
					<h4>The connection to the database was <b>NOT</b> successful.</h4>
				<?php elseif($step == 2):?>
					<h4>The following error occured while attempting to create your database.</h4>
				<?php elseif($step == 3):?>
					<h4>The FTP test was <b>NOT</b> successful.</h4>
				<?php endif;?>
				
				<p><br /><b><?=$dbError;?></b></p>
				
				<?php if($connectError):?>
					<p>Please backup and review your database server settings.</p>
					<p>If you require additional help with your specific server settings, please consult your hosting company.</p>
				<?php endif;?>
				
				<p align="center"><form><input type="button" name="button" value="Backup" onClick="javascript:history.go(-1);"></form></p>
			</td>
		</tr>
	</table>

<?php else:?>

	<?php if($step == 1):?>
	
		<h4><br />&nbsp;</h4>

		<table border="0" cellpadding="2" width="750">
			<tr>
				<td colspan="2">
					<h4>Step One - Setting up Your MySQL Database</h4>
				</td>
			</tr>
			<tr>
				<td colspan="2">
					These parameters are provided by your hosting company.
					If you do not know what these settings are, you need to
					contact them before you can continue.
				</td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td colspan="2">
					Please enter the following parameters:
				</td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td align="right">MySQL Host Name: </td>
				<td width="60%"><input size="30" type="text" name="db_host" value="localhost"></td>
			</tr>
			<tr>
				<td align="right">Your Database Name: </td>
				<td><input size="30" type="text" name="db_name" value=""></td>
			</tr>
			<tr>
				<td align="right">Your Username for This Database: </td>
				<td><input size="30" type="text" name="db_username" value=""></td>
			</tr>
			<tr>
				<td align="right">Your Password for This Database: </td>
				<td><input size="30" type="password" name="db_password" value=""></td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr><td colspan="2"><b>NOTE:</b> We do not install the 3.0 tables in the database if they already exist.</td></tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td align="right">Remove all tables in database before installing: </td>
				<td>
					<select name="delete_tables">
						<option value="no">No</option>
						<option value="yes">Yes<option>
					</select>
				</td>
			</tr>
			<tr>
				<td align="right">Add 3.0 tables to existing tables in the database: </td>
				<td>
					<select name="add_tables">
						<option value="no">No</option>
						<option value="yes">Yes<option>
					</select>
				</td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td colspan="2" align="center">
					<input type="hidden" name="step" value="2">
					<input type="submit" name="continue" value="Continue">
				</td>
			</tr>
			
		</table>

	<?php elseif($step == 2):?>

		<script type="text/javascript">
			alert("Your QuikStore 3.0 database was created successfully...");
		</script>

		<table border="0" cellpadding="3" width="600">
			<tr>
				<td colspan="2">
					<h4>Step Two - Setting up the FTP Parameters</h4>
					<p>These parameters define the ftp settings for your store and are used to upload files to your server.</p>
				</td>
			</tr>
			<tr>
				<td colspan="2">
					These parameters are provided by your hosting company.
					If you do not know what these settings are, you need to
					contact them before you can continue.
				</td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td align="right">FTP Host: </td>
				<td width="60%"><input size="50" type="text" name="ftp_host" value="<?=$_SESSION['ftp_host'];?>"></td>
			</tr>
			<tr>
				<td align="right">FTP Username: </td>
				<td width="60%"><input size="50" type="text" name="ftp_username" value="<?=$_SESSION['ftp_username'];?>"></td>
			</tr>
			<tr>
				<td align="right">FTP Password: </td>
				<td width="60%"><input size="50" type="password" name="ftp_password" value="<?=$_SESSION['ftp_password'];?>"></td>
			</tr>
			<tr>
				<td align="right">Use Passive Mode: </td>
				<td width="60%"><input type="checkbox" name="ftp_passive" value="true"></td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td colspan="2" align="center">
					<input type="hidden" name="step" value="3">
					<input type="submit" name="continue" value="Continue">
				</td>
			</tr>
        </table>

	<?php elseif($step == 3):?>

		<table border="0" cellpadding="2" cellspacing="0" width="600">
			<tr>
				<td colspan="2">
					<h4>Step Three - Selecting the store FTP document root</h4>
					<p>
						Click on the links below to find the directory where you have the QuikStore 3.0 files installed.<br /><br />
						Once you find the directory, click the radio button next to it and then click the continue button at the bottom of the screen.
					</p>
				</td>
			</tr>
			<tr><td>&nbsp;</td></tr>
			
			<?php if(count($docRootList) > 0):?>
			<tr>
				<td nowrap style="border:1px solid #A9A9A9;">
					<p><b>Current Directory:</b> <?=$ftpcwd;?><br /></p>
					<div style="overflow:auto; height:500px; width:600px;padding-top:10px;">
						<?php foreach($docRootList as $j=>$dir):?>
						
							<?php if($dir == $ftpcwd):?>
								<input type="radio" name="ftp_document_root" value="<?=$dir;?>" checked>
								<span style="background:yellow;"><?=$dir;?></span><br />
							<?php else:?>
								<!-- input type="radio" name="ftp_document_root" value="<?=$dir;?>" -->
								<?php if($ftpcwd != '/'):?>
									<a href="install.php?step=3&move=<?=$ftpcwd;?>/<?=$dir;?>"><?=$dir;?></a><br />
								<?php else:?>
									<a href="install.php?step=3&move=/<?=$dir;?>"><?=$dir;?></a><br />
								<?php endif;?>
							<?php endif;?>
						<?php endforeach;?>
						<p>&nbsp;</p>
					</div>
				</td>
			</tr>
			<?php else:?>
			<tr>
				<td>
					<p>The server did not return a list of directories. Please enter the directory path below:</p>
					<input size="50" type="text" name="ftp_document_root" value="">
				</td>
			</tr>
			<?php endif;?>
			<tr><td>&nbsp;</td></tr>
			<tr>
				<td>
					<input type="submit" name="continue" value="Continue">
				</td>
			</tr>
		</table>
		<input type="hidden" name="step" value="4">

	<?php elseif($step == 4):?>

		<script type="text/javascript">
			alert("Your FTP settings tested successfully...");
		</script>
		
		<table border="0" cellpadding="3" width="600">
			<tr>
				<td colspan="2">
					<h4>Step Three - Setting up the initial store</h4>
				</td>
			</tr>
			<tr>
				<td colspan="2">
					These parameters define the initial settings for your store. Try not to leave
					any of them blank if you can. You can always modify them in the control panel later.
				</td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td colspan="2">
					Please enter the following parameters:
				</td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<th colspan="2" align="left">Basic Store Parameters</th>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td align="right" width="30%">Your Store Name: </td>
				<td width="70%"><input size="50" type="text" name="store_name" value=""></td>
			</tr>
			<tr>
				<td align="right">URL to the Store: </td>
				<?php if(!empty($_SERVER['HTTP_HOST'])):?>
					<td>
						<input size="50" type="text" name="web_site_url" value="http://<?=$_SERVER['HTTP_HOST'];?>"> </td>
						</tr> <tr><td> </td>
						<td>If you are installing to a subdirectory, you must append this to the URL and Secure URL - eg. http://www.quikstore.com/shop
						
					</td>
				<?php else:?>
					<td>
						<input size="50" type="text" name="web_site_url" value="http://">
					</td>
				<?php endif;?>
			</tr>
			<tr>
				<td align="right">(optional) Secure URL: </td>
				<td><input size="50" type="text" name="secure_url" value=""></td>
			</tr>
			
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<th colspan="2" align="left">
					Store Email Parameters
				</th>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td align="right">Store owner email address: </td>
				<td><input size="50" type="text" name="store_email_address" value=""></td>
			</tr>
			<tr>
				<td align="right" nowrap>Email server name or IP Address: </td>
				<td><input size="50" type="text" name="mail_server_host" value=""></td>
			</tr>
			<tr>
				<td align="right">Email server username: </td>
				<td><input size="50" type="text" name="mail_server_username" value=""></td>
			</tr>
			<tr>
				<td align="right">Email server password: </td>
				<td><input size="50" type="password" name="mail_server_password" value=""></td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<th colspan="2" align="left">
					Admin Parameters (for Control Panel)
				</th>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td align="right">New Admin password: </td>
				<td><input size="50" type="password" name="admin_password" value=""></td>
			</tr>
			<tr>
				<td align="right">Admin email address: </td>
				<td><input size="50" type="text" name="admin_email" value=""></td>
			</tr>
			<tr>
				<td>&nbsp;</td>
				<td>
				The security phrase below is used to encrypt the credit card details stored in the database.
				</td>
			</tr>
			<tr>
				<td align="right">Security Phrase: </td>
				<td><input size="50" type="text" name="security_phrase" value=""></td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
			<tr>
				<td colspan="2" align="center">
					<input type="hidden" name="step" value="5">
					<input type="submit" name="continue" value="Continue">
				</td>
			</tr>
		</table>

	<?php elseif($step == 5):?>

		<h4><br />Congratulations! Your basic store parameters have been set...</h4>
		<p><font color="red"><b>NOTE: </b>It is strongly recommended that you remove this install.php file from your server so that it cannot be run again!</font></p>
		<p><a href="<?="$cpanelURL/index.html"?>">Click here</a> to login to the control panel</p>

	<?php endif;?>

<?php endif;?>

</form>
</div>
</body>
</html>
