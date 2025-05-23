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

include_once("../include/ftp.inc");
$ftp = new Ftp();
$ftpRoot = $_CF['ftp']['document_root'] . "/cp";
$ftp->ChDir($ftpRoot);

$quote = '"';
$sqlResult = null;
$strsql = null;

$optimizeResults = array();

chdir("../");
$cwd = getcwd();
$cwd = str_replace('\\','/',$cwd);
$today = date("mdy.g.ia");

if(!is_dir("$cwd/backups")){
	if($ftp->Mkdir($ftpRoot,$ftpRoot . "/backups", 755)){
		$sourceData = "<Limit GET POST>$_CR";
		$sourceData .= "deny from all$_CR";
		$sourceData .= "</Limit>$_CR";
		$ftp->writeFile("$ftpRoot/backups/.htaccess",$sourceData);
	}
	else{
		exit($ftp->error);	
	}
}
else{
	if(!file_exists("$cwd/backups/.htaccess")){
		$sourceData = "<Limit GET POST>$_CR";
		$sourceData .= "deny from all$_CR";
		$sourceData .= "</Limit>$_CR";
		$ftp->writeFile("$ftpRoot/backups/.htaccess",$sourceData);
	}
}
$taskType = "Backup";

if(!empty($_REQUEST['continue'])){

	if(!empty($_REQUEST['task']) && $_REQUEST['task'] == 'backup'){

		$dbFile = "$ftpRoot/backups/$today.sql";
		$schemaFile = "$ftpRoot/backups/" . DB_NAME . ".schema.sql";
		
		$data = array();
		$line = null;
		
		$sql = "SHOW TABLES FROM `" . DB_NAME . "`";
		$schemas = array();
		$rs = $_DB->execute($sql);
		
		while ($row = mysql_fetch_row($rs)) {
		
			$name = trim($row[0]);
			
			if($name == "updates"){
				continue;	
			}
			
			$schema = $_DB->getRecord("SHOW CREATE TABLE `$name`");
			$schemas[] = $schema['create table'];

			$pkey = null;
			$rsKeys = $_DB->execute("SHOW KEYS FROM `$name`");
			while($keys = $_DB->fetchrow($rsKeys,'ASSOC')){
				if($keys['key_name'] == "PRIMARY"){
					$pkey = $keys['column_name'];
					break;
				}
			}
			
			if(!empty($_REQUEST['overwrite']) && $_REQUEST['overwrite'] == 'true'){
				if($name != 'sessions'){
					$data[] = "TRUNCATE `$name`;";
				}
			}
			$data[] = "OPTIMIZE TABLE `$name`;";
			
			$rsData = $_DB->execute("SELECT * FROM `$name`");
			if($_DB->numrows($rsData) > 0){
				$strNames = null;
				$aryValues = array();
				while($row = $_DB->fetchrow($rsData,'ASSOC')){
					foreach($row as $k=>$v){
						if(trim($v) == "" && !empty($defaultVals[$k])){
							$row[$k] = $defaultVals[$k]; 
						}
						$row[$k] = str_replace("'","''",$row[$k]);
					}
					if(!$strNames){
						$strNames = "`" . join("`,`",array_keys($row)) . "`";
					}
					$aryValues[] = "('" . join("','",array_values($row)) . "')";

				}
				$strValues = join(",$_CR",$aryValues);
				if($name != 'sessions'){
					if($pkey){
						$strLine = "INSERT INTO `$name` ($strNames) VALUES $strValues ON DUPLICATE KEY UPDATE `$pkey` = LAST_INSERT_ID(`$pkey`);";
					}
					else{
						$strLine = "INSERT INTO `$name` ($strNames) VALUES $strValues;";
					}
					$data[] = $strLine;
				}
			}
		}
		
		$strData = join($_CR,$data);
		
		$ftp->writeFile($dbFile,$strData);
		$ftp->Chmod($dbFile,0644);
		
		// write the table info
		$rows = null;
		foreach($schemas as $j=>$txt){
			$rows .= "$txt;$_CR$_CR";
		}
		$rows .= $_CR;
		$ftp->writeFile($schemaFile,$rows);
		
		$ftp->Close();
	}
	elseif(!empty($_REQUEST['task']) && !empty($_REQUEST['filetorestore']) && $_REQUEST['task'] == 'restore'){
		$fName = $_REQUEST['filetorestore'];
		if(file_exists("$cwd/backups/$fName")){
			run_query_batch($_DB->cnx, "$cwd/backups/$fName");
		}
		$taskType = "Restore";
	}
	elseif(!empty($_REQUEST['task']) && $_REQUEST['task'] == 'optimize'){
		optimizeTables();
		$taskType = "Optimize";
	}
	elseif(!empty($_REQUEST['task']) && $_REQUEST['task'] == 'runsql' && trim($_REQUEST['strsql']) != ""){
		$strsql = stripslashes(trim(stripslashes($_REQUEST['strsql'])));
		//$_Common->debugPrint($strsql);
		if(strtolower(substr($strsql,0,6)) == "select" || strtolower(substr($strsql,0,4)) == "show"){
			$sqlResult = $_DB->getRecords($strsql);
		}
		else{
			$rs = $_DB->execute($strsql);
		}
		$taskType = "Run SQL";
	}
}

function run_query_batch($handle, $filename){ 

	global $_Common,$_CR;

	// -------------- 
	// Open SQL file. 
	// -------------- 
	if (!($fd = fopen($filename, "r")) ) { 
		exit("Failed to open $filename for import"); 
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
		
		if((empty($_REQUEST['overwrite']) || $_REQUEST['overwrite'] != 'true') && substr($line,0,8) == "TRUNCATE"){
			continue;
		}
		 
		$stmt = "$stmt$line"; 

		// ------------------------------------------------------------------- 
		// Semicolon indicates end of statement, keep adding to the statement. 
		// until one is reached (at the end of a line). 
		// ------------------------------------------------------------------- 
		if (substr(trim($stmt),-1) != ';') { 
			continue; 
		} 

		// ---------------------------------------------- 
		// Remove semicolon and execute entire statement. 
		// ---------------------------------------------- 
		$stmt = str_replace(";", "", $stmt); 

		// ---------------------- 
		// Execute the statement. 
		// ---------------------- 
		// $_Common->debugprint($stmt);
		mysql_query($stmt, $handle) || exit("Query failed: " . mysql_error()); 
		$stmt = ""; 
	} 

	// --------------- 
	// Close SQL file. 
	// --------------- 
	fclose($fd); 
}

// -------------------------------------------------------------------
function optimizeTables(){
	
	global $_Common,$optimizeResults;
	
	$sql = "SHOW TABLES FROM `" . DB_NAME . "`";
	$result = mysql_query($sql);
	if(!$result) {
		print "DB Error, could not list tables\n";
		print 'MySQL Error: ' . mysql_error();
		exit;
	}
	while($row = mysql_fetch_row($result)) {
		
		$table = $row[0];
		
		$opt = mysql_query("OPTIMIZE TABLE `$table`");
		if(!$opt) {
			$optimizeResults[] = "\t<font color=red>DB Error: " . mysql_error() . "</font>\n";
			$optimizeResults[] = "\t<font color=red>Could not optimize table: $table. Will attempt to repair.</font>\n";
			// bad table, try to repair it
			if(mysql_errno($link) == 144 || strstr(mysql_error($link),'errno: 144')){
				$repaired = @mysql_query("REPAIR TABLE `$table`");
				if(!$repaired) {
					$optimizeResults[] = "\t<font color=red>DB Error: " . mysql_error() . "</font>\n";
					$optimizeResults[] = "\t<font color=red>Could not repair table: $table.</font>\n";
				}
				else{
					$optimizeResults[] = "<font color=blue>$table has been repaired.</font>\n";
				}
				@mysql_free_result($repaired);
			}
		}
		else{
			$optimizeResults[] = "$table has been optimized.\n";
		}
		@mysql_free_result($opt);
	}
	@mysql_free_result($result);
}
?>
<html>
<head>
<title>Database backup</title>
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
document.write('<link rel="stylesheet" href="../stylesheets/' + styles + '" type="text/css">');

// -------------------------------------------------------------------
function showIt(whichEl,show){
    if(document.all){
        whichEl = document.all[whichEl];
    }
    else{
        whichEl = document.getElementById(whichEl);
    }
    if(show){
		whichEl.style.display = "";
	}
	else{
		whichEl.style.display = "none";
	}
}

<?php if(isset($_REQUEST['task'])):?>
function selectOptions(form){
	var selectedTask = '<?=$_REQUEST['task'];?>';
	for (i=0;i<form.elements['task'].length; i++) {
		if (form.elements['task'][i].value == selectedTask) {
			form.elements['task'][i].checked = true;
			break;
		}
	}
}
<?php endif;?>

//-->
</script>
<style>
td{
	vertical-align:middle;
}
.grey{
	background-color:E5E5E5;
}
</style>
</head>
<body class="mainBody">

	<div align="center">

		<form action="30.database.php" method="get">
			<table border="0" cellpadding="1" cellspacing="0" width="600">
				<tr>
					<th colspan="2" align="left">Database Utilities for: &quot;<?=DB_NAME;?>&quot;</th>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				<tr>
					<td colspan="2" align="left"><b>This function backs up your QuikStore database to an .sql file in the cp/database directory.</b></td>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				<tr>
					<td align="right" width="30%">Run Database Backup: </td>
					<td width="70%"><input type="radio" name="task" value="backup" checked onFocus="this.checked = true;showIt('restore',false)"></td>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				<tr>
					<td colspan="2" align="left"><b>This function restores a QuikStore 3.0 database .sql file from the cp/database directory.</b></td>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				<tr>
					<td align="right">Run Restore: </td>
					<td><input type="radio" name="task" value="restore" onFocus="this.checked = true;showIt('restore',true)"></td>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				
				<tr id="restore" style="display:none;">
					<td colspan="2" style="border:1 dashed #A9A9A9;padding:10px;">
					
						<table border="0" cellpadding="1" cellspacing="0" width="100%">
							<tr>
								<td colspan="2" align="left"><b>Select a QuikStore 3.0 database file from the cp/database directory to restore.</b></td>
							</tr>
							<tr>
								<td colspan="2">&nbsp;</td>
							</tr>
							<tr>
								<td align="right" width="30%">File to Restore: </td>
								<td width="70%">
									<?php
										$handle = opendir("$cwd/backups");
										$files = array();
										while ($file = readdir($handle)){
											if($file != "." && $file != ".." && substr($file,0,1) != "_"){
												$pathinfo = pathinfo($file);
												if(!empty($pathinfo['extension']) && $pathinfo['extension'] == "sql"){
													$files[] = $file;
												}
											}
										}
										closedir($handle);
										if(count($files) > 0){
											$select = $_Common->makeSimpleSelectBox('filetorestore',$files,$files);
										}
										else{
											$select = "No restore files found";	
										}
									?>
									<?=$select;?>
								</td>
							</tr>
							<tr>
								<td colspan="2">&nbsp;</td>
							</tr>
							<tr>
								<td colspan="2" align="left"><b>Should we overwrite the existing data in the QuikStore 3.0 database during the restore?</b></td>
							</tr>
							<tr>
								<td colspan="2">&nbsp;</td>
							</tr>
							<tr>
								<td align="right">Overwite during restore:</td>
								<td><input type="checkbox" name="overwrite" value="true" checked></td>
							</tr>
							<tr>
								<td colspan="2">&nbsp;</td>
							</tr>
						</table>
					</td>
				</tr>


				<tr>
					<td colspan="2" align="left"><b>This function allows you to optimize/repair tables in the &quot;<?=DB_NAME;?>&quot; database.</b></td>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				<tr>
					<td align="right">Optimize/Repair Tables: </td>
					<td><input type="radio" name="task" value="optimize" onFocus="this.checked = true;showIt('restore',false)"></td>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				
				
				<tr>
					<td colspan="2" align="left"><b>This function allows you to run sql commands on the &quot;<?=DB_NAME;?>&quot; database.</b></td>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				<tr>
					<td align="right">Run SQL: </td>
					<td><input type="radio" name="task" value="runsql" onFocus="this.checked = true;showIt('restore',false)"></td>
				</tr>
				<tr style="padding-top:10px;">
					<td align="right" style="vertical-align:top;">SQL Command: </td>
					<td>
						<textarea name="strsql" rows="10" cols="60" wrap="virtual"><?=$strsql;?></textarea>
					</td>
				</tr>
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				<tr>
					<td colspan="2" align="center"><input type="submit" name="continue" value="Continue"></td>
				</tr>
			</table>
		</form>
		
		<?php if(!empty($_REQUEST['continue'])):?>

			<?php if(!is_null($_DB->error)):?>

				<p><b><font color=red><?=$taskType;?> failed</font></b></p>

			<?php else:?>

				<p><font color=blue><b><?=$taskType;?> complete.</b></font></p>
				<?php if($taskType == "Backup"):?>
					<p>The sql files can be found in: <?="$cwd/backups/$today.sql";?></p>

				<?php elseif($taskType == "Optimize" && count($optimizeResults) > 0):?>
					<table border="0" cellpadding="1" cellspacing="0" width="600">
					<?php foreach($optimizeResults as $i=>$t):?>
						<tr><td><?=$t;?></td></tr>
					<?php endforeach;?>
					</table>
				<?php endif;?>

				<?php
					if($sqlResult){
						$recordCount = count($sqlResult);
						print "<div align=\"left\"><p><b>SQL Result: ($recordCount records)</b></p></div>";
						$_Common->debugPrint($sqlResult);
					}
				?>


			<?php endif;?>

		<?php endif;?>

	</div>


<?php if(isset($_REQUEST['task'])):?>
<script LANGUAGE="JavaScript">
selectOptions(document.forms[0]);
</script>
<?php endif;?>

</body>
</html>




