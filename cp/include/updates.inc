<?php
class Updates{

	var $debug = false;
	var $error = null;
    var $root = null;
    var $files = array();
    var $filesToUpdate = array();
    var $newFiles = array();
    var $newGateways = array();
    var $passive = true;
    var $response_meta_info = null;
    var $result = null;
    var $success = false;
    var $updateFilename = null;
    var $updatedFiles = array();
    var $validExtensions = array("css","html","inc","js","php");
    var $exclude = array("config","startup","store.template","install.php");
    
    // --------------------------------------------------------------
    function __construct(){

		// backup to the store root from the utilities directory
		chdir("../../");
		$cwd = getcwd();
		$cwd = str_replace("\\","/",$cwd);
		$this->root = $cwd;
		
		if(empty($_SESSION['update_table_done'])){
			$this->checkTable();
		}
		
		if(isset($_REQUEST['passive'])){
			$this->passive = true;
			$_SESSION['passive'] = true;
		}
		elseif(isset($_SESSION['passive'])){
			$this->passive = $_SESSION['passive'];
		}
		elseif(empty($_REQUEST['passive']) && empty($_SESSION['passive'])){
			$this->passive = false;
			$_SESSION['passive'] = false;
		}
		
		global $_Config,$_DB,$_Registry;
		if($path = $_Registry->file_exists_incpath("db.updates.inc")){
			// do any database updates
			include_once("db.updates.inc");	
		}
	}
	
	// --------------------------------------------------------------
	function checkVersions(){
		
		global $_Common;
		global $_Registry;

		// a little sanity check
		if(!empty($_REQUEST['extensions'])){
			foreach($_REQUEST['extensions'] as $j=>$ext){
				if(!in_array($ext,$this->validExtensions)){
					unset($_REQUEST['extensions'][$j]);
				}
			}
			if(count($_REQUEST['extensions']) > 0){
				$this->validExtensions = $_REQUEST['extensions'];
			}
		}

		$this->validExtensions[] = "sql";


		// this code contacts the quikstore server to 
		// get the checksums for the different files and
		// then compares them to the local versions
		
		$url = "http://www.quikstore.com/get.update.list.php";
		
		$ch = curl_init($url);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		$strList = curl_exec($ch);
		curl_close($ch);

		if(stristr($strList,"error ")){
			$strList = null;
			if(!($H = fopen("$url","r"))){
				print "Could not connect to Quikstore.com. Please back up and try again.";
				exit;
			}
			while(!feof($H)){
				$strList .= fgets($H,1024);	
			}
			fclose($H);
		}

		if($this->debug){
			$_Common->debugPrint($strList,"Get Update List result");
		}
		
		$list = explode("\n",$strList);
		$masterList = array();
		foreach($list as $i=>$pair){
			if(!strstr($pair,'=')){
				continue;	
			}
			@list($key,$val) = explode('=',$pair);
			if(trim($key) != "" && trim($val) != ""){
				$md5s = explode(',',trim($val));
				$masterList[trim($key)] = $md5s;
			}
		}

		// now get the local files and compare
		$this->getDirArray($this->root);
		ksort($this->files);
		
		if($this->debug){
			$_Common->debugPrint($masterList,"Master List");
			$_Common->debugPrint($this->files,"local list");
		}

		foreach($masterList as $fileName=>$md5){
			
			$exclude = false;
			foreach($this->exclude as $l=>$test){
				if(stristr($fileName,$test)){
					$exclude = true;
					break;
				}
			}
			if($exclude){
				continue;	
			}
			
			if($this->debug && isset($this->files[$fileName])){
				$debugList[$fileName] = array($md5,$this->files[$fileName]); 
			}
			
			if(!isset($this->files[$fileName]) && $this->testExtension($fileName)){
				$this->newFiles[$fileName] = $md5;
			}	
			elseif(isset($this->files[$fileName])){
				
				$found = false;
				foreach($md5 as $k=>$sum){
					if(in_array($sum,$this->files[$fileName])){
						$found = true;
						break;
					}
				}
				if(!$found){
					$this->filesToUpdate[$fileName] = $md5;
				}
			}
		}
		
		$_SESSION['update_files'] = array_merge($this->newFiles,$this->filesToUpdate);

		if($this->debug){
			$_Common->debugPrint($_SESSION['update_files'],"Debug List");
		}
	
	}
	
	// --------------------------------------------------------------
	function testExtension($filename){
		foreach($this->validExtensions as $i=>$ext){
			if(substr(basename($filename),-strlen($ext)) == $ext){
				return true;	
			}
		}
		return false;
	}


	// --------------------------------------------------------------
	function getFileList(){
		
		$dlFiles = array();
		if(!empty($_REQUEST['files']) && count($_REQUEST['files']) > 0){
			foreach($_REQUEST['files'] as $i=>$fpath){
				if(isset($_SESSION['update_files'][$fpath])){
					$dlFiles[] = $fpath;
				}
			}
		}
		return $dlFiles;
	}

	
	// --------------------------------------------------------------
	// download a list of files
	function downloadFiles(){

		global $_Common, $_CF, $_Registry;

		$email = trim($_REQUEST['email_address']);

		$files = $this->getFileList();

		if($this->debug){
			$_Common->debugPrint($files);
			flush();
		}

		if(count($files) > 0){

			$fileStr = join(',',$files);

			$url = "http://www.quikstore.com/get.update.file.php";
			$request = "email=$email&files=$fileStr";
			$info = array();

			// post the list of files
			$ch = curl_init($url);
			if($this->debug){
				curl_setopt($ch, CURLOPT_VERBOSE, true);
			}
			curl_setopt($ch, CURLOPT_HEADER, false);
			curl_setopt($ch, CURLOPT_HEADERFUNCTION, array(&$this,'readHeader'));
			curl_setopt($ch, CURLOPT_TIMEOUT, 20);
			curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
			curl_setopt($ch, CURLOPT_POST, true);
			curl_setopt($ch, CURLOPT_POSTFIELDS, $request);
			$result = curl_exec($ch);
			$info = curl_getinfo($ch);
			curl_close($ch);

			//add the headers from the custom headers callback function
			$info = array_merge($info, $this->response_meta_info);

			if(stristr($result,"error ")){
				$_Common->debugPrint($result,"cURL file download error");
				exit;
			}

			$today = date("Ymd.His");
			
			$_SESSION['timestamp'] = date("Y-m-d H:i:s");
			
			$updateFileName = "$today.updates.zip";
			if(!empty($info['content_disposition'])){
				$updateFileName = $info['content_disposition'];
			}
			$_SESSION['today'] = $today;
			$_SESSION['update_filename'] = $updateFileName;
			
			if($this->debug){
				$_Common->debugPrint($info,"curl info");
				flush();
			}
			
			// write new file
			if($result != "" && substr($result,0,6) != "Error:"){
				$_Registry->LoadClass("ftp");
				$ftp = new Ftp();
				$ftpRoot = $_CF['ftp']['document_root'];
				$ftp->ChDir($ftpRoot);

				// turn passive mode on
				ftp_pasv($ftp->connection, $this->passive);

				$temp = tmpfile();
				fwrite($temp,$result);
				fseek($temp,0);
				error_reporting(E_PARSE|E_ERROR);
				ftp_fput($ftp->connection, "cp/database/$updateFileName", $temp, FTP_BINARY);
				error_reporting(E_ALL);
				fclose($temp);
				$ftp->close();
				
				if($ftp->error){
					$this->error = $ftp->error;
				}
				else{
					$this->success = true;
					$this->updateFilename = $updateFileName;
				}
			}
			elseif(substr($result,0,6) == "Error:"){
				$this->error = $result;
			}
			else{
				$this->error = "No results were returned.";
			}
		}
		else{
			$this->error = "No valid files were selected to download.";
		}
	}

	// --------------------------------------------------------------
	function readHeader($ch, $header) {
		//extracting example data: filename from header field Content-Disposition
		$filename = $this->extractCustomHeader('Content-Disposition: attachment; filename=', '\n', $header);
		if ($filename) {
			$this->response_meta_info['content_disposition'] = trim($filename);
		}
		return strlen($header);
	}

	// --------------------------------------------------------------
	function extractCustomHeader($start,$end,$header) {
		$pattern = '/'. $start .'(.*?)'. $end .'/';
		if (preg_match($pattern, $header, $result)) {
			return $result[1];
		} else {
			return false;
		}
	}
	
	// --------------------------------------------------------------
	function unzipFiles(){
		
		global $_Common, $_CF, $_Registry;
		
		$updateFileName = $_SESSION['update_filename'];
		
		if(file_exists("cp/database/$updateFileName")){
			
			include_once('cp/include/pclzip.lib.php');
			$archive = new PclZip("cp/database/$updateFileName");

			$list = $archive->extract(PCLZIP_OPT_EXTRACT_AS_STRING);
			if($list == 0) {
				$this->error = $archive->errorInfo(true);
			}
			else{
				$_Registry->LoadClass("ftp");
				$ftp = new Ftp();
				$ftpRoot = $_CF['ftp']['document_root'];
				$ftp->ChDir($ftpRoot);

				// turn passive mode on
				ftp_pasv($ftp->connection, $this->passive);

				foreach($list as $j=>$props){
					$fName = $props['filename'];
					$this->backupFile($fName);
					$content = $props['content'];
					
					$dirname = dirname($fName);
					if(!file_exists($dirname)){
						ftp_mkdir($ftp->connection,$dirname);
					}
					
					$temp = tmpfile();
					fwrite($temp,$content);
					fseek($temp,0);
					ftp_fput($ftp->connection, $fName, $temp, FTP_BINARY);
					fclose($temp);
					$this->updatedFiles[] = $fName;
				}
				$ftp->close();
				$this->success = true;
			}
			
			$today = $_SESSION['today'];
			
			
		}
		else{
			$this->error = "The update file: cp/database/$updateFileName does not exist or is not readable";
		}
	}

	// --------------------------------------------------------------
	function backupFile($file){
		
		global $_Common,$_DB;

		$tStamp = $_SESSION['timestamp'];

		$sql = "SELECT `file_name` FROM updates WHERE `file_name` = '$file' AND backup_date = '$tStamp'";
		$found = $_DB->getRecord($sql);

		if(count($found) == 0 && file_exists($file)){

			$content = null;
			$h = fopen($file, "rb");
			while(!feof($h)){
				$content .= fread($h,1024);
			}
			fclose($h);
			$content = addslashes($content);

			$sql = "INSERT INTO updates (`file_name`,`contents`,`backup_date`) VALUES ('$file','$content','$tStamp')";
			$_DB->execute($sql);
		}
		
		// delete the oldest version of the file if there are more than 2 backup files. 
		// Keep the last 2 backups. Otherwise the database could get huge very quickly.
		$sql = "SELECT upid,file_name,backup_date FROM updates WHERE `file_name` = '$file' ORDER BY backup_date";
		$rs = $_DB->execute($sql);
		$oldFiles = array();
		while($row = $_DB->fetchrow($rs, "ASSOC")){
			$oldFiles[] = $row;
		}
		$_DB->free_result($rs);
	
		$fileCount = count($oldFiles) - 2;
		
		if($this->debug){
			$_Common->debugPrint($oldFiles,"$fileCount old files");
		}
		
		if($fileCount > 0){
			for($i=0;$i<$fileCount;$i++){
				$upid = $oldFiles[$i]['upid'];
				$sql = "DELETE FROM updates WHERE `upid` = '$upid'";
				if($this->debug){
					$_Common->debugPrint($sql);
				}
				$rs = $_DB->execute($sql);
				$_DB->free_result($rs);
			}	
		}
	}

	// --------------------------------------------------------------
	// gets a list of the files and their md5 checksum for the compare
	function getDirArray($sPath){

		global $files,$root;

		$root = $this->root;

		$retVal = array();

		//Load Directory Into Array
		$handle=opendir($sPath);
		while ($file = readdir($handle)){
			$retVal[count($retVal)] = $file;
		}
		closedir($handle);
		
		while(list($key, $val) = each($retVal)){
			if($val != "." && $val != ".." && substr($val,0,1) != "_"){
				$path = "$sPath/$val";
				if(is_file($path) && $this->getFileExtension($val)){
					$shortPath = str_replace("$root/","",$path);
					
					// put the md5 result in both ways so we don't need to
					// worry about carriage returns in windows servers.
					// both versions get tested on line 108
					$txt = trim(file_get_contents($path));
					$txt = str_replace("\r","",$txt);
					$this->files[$shortPath][] = md5($txt);
					$this->files[$shortPath][] = md5_file($path);
				}
				if(is_dir($path) && !strstr($path,"backups")){
					$this->getDirArray($path);
				}
			}
		}
	}
	
	// --------------------------------------------------------------
	function getFileExtension($filename){

		$pathinfo = pathinfo($filename);
		if(!empty($pathinfo['extension']) && in_array($pathinfo['extension'],$this->validExtensions)){
			return true;	
		}
		return false;
	}
	// --------------------------------------------------------------
	function checkTable(){
		global $_DB;
		if(!$_DB->mysql_table_exists("updates")){
			$sql = "CREATE TABLE `updates` (`upid` int(10) NOT NULL auto_increment,`file_name` text,`contents` blob,`backup_date` datetime default NULL, PRIMARY KEY  (`upid`)) ENGINE=MyISAM";
			$_DB->execute($sql);
			$_SESSION['update_table_done'] = true;
		}
	}

	// --------------------------------------------------------------
	function checkGateways(){
		
		global $_Common, $_DB;
		
		$data = $_DB->getRecords("SELECT gateway_name FROM payment_gateways ORDER BY gateway_name",'gateway_name');
		$this->validExtensions = array('inc');
		$this->getDirArray($this->root . '/include/gateways');
		
		foreach($this->files as $path=>$md5s){
			$fName = basename($path);
			$className = str_replace('.inc','',$fName);
			if(!isset($data[$className])){
				if((@include_once("include/gateways/$fName")) !== false){ 
					$class = new $className();
					if(method_exists($class,'loadDefaults')){
						$class->loadDefaults();
						//$_Common->debugPrint($className . " loaded");
						$this->newGateways[] = $fName;
					}
				}
			}
		}
	}
}
?>