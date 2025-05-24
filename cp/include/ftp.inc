<?php

class Ftp{

	public $connection = null;
	public bool $debug = false;
	public $error = null;
    public string $version = "3.0.0";
    
    // ----------------------------------------------------------------
    function __construct(){
		global $_Common;
		if(!$this->Connect() || $this->error){
			$_Common->printErrorMessage("FTP Connection failed",$this->error);
		}
	}

	// ----------------------------------------------------------------
	function Connect(){
		
		global $_CF;

		$server = $_CF['ftp']['ftp_host']; // ftp server
		 
		$connection = ftp_connect($server,21,30); // connection 

		// check if connection was made 
		if(!$connection){
			$this->error = "Could not connet to $server";
			return false;
		}

		// login to ftp server 
		$user = $_CF['ftp']['username']; 
		$pass = $_CF['ftp']['password']; 
		$login = ftp_login($connection, $user, $pass);

		// check if login ok 
		if(!$login){
			if($connection){
				ftp_close($connection); // close connection 
			}
			$this->error = "Could not login to $server with the supplied username and password";
			return false; 
		}
		else{
			$this->connection = $connection;
		}
		
		return true;
	}
	
	// ----------------------------------------------------------------
	function setPassiveMode($on = false){
		ftp_pasv($this->connection, true);
	}
	
	// ----------------------------------------------------------------
	function Close(){
		if($this->connection){
			ftp_close($this->connection); // close connection
			$this->connection = null;
		}	
	}

	// ----------------------------------------------------------------
	function updateFile($destPath,$sourceData,$isImage = false){

		$temp = tmpfile();
		fwrite($temp,$sourceData);
		fseek($temp,0);
		if(!$this->UploadFile($temp,$destPath,$isImage)){
			$this->Close();
			fclose($temp);
			exit($this->error);
		}	
		fclose($temp);
	}
	
	// ----------------------------------------------------------------
	function writeFile($destPath,$sourceData,$isImage = false){
		$this->updateFile($destPath,$sourceData,$isImage = false);
	}

	// ----------------------------------------------------------------
	function Upload($src,$dest,$isImage = false){
		
		$mode = FTP_ASCII;
		if($isImage){
			$mode = FTP_BINARY;
		}
		
		if(!ftp_put($this->connection, $dest, $src, $mode)) {
			$this->error = "FTP Upload failed";
			return false;
		}
		if($isImage){
			$this->Chmod($dest,755);
		}
		
		
		return true;
	}

	// ----------------------------------------------------------------
	function UploadFile($fh,$dest,$isImage = false){
		
		global $_Common;
		
		$mode = FTP_ASCII;
		if($isImage){
			$mode = FTP_BINARY;
		}
		
		if(!ftp_fput($this->connection, $dest, $fh, $mode)) {
			$this->error = "FTP Upload failed for: $dest";
			return false;
		}
		if($isImage){
			$this->Chmod($dest,755);
		}
		
		return true;
	}

	// ----------------------------------------------------------------
	function Download(){

	}

	// ----------------------------------------------------------------
	function Cwd(){
		return ftp_pwd($this->connection); 	
	}

	// ----------------------------------------------------------------
	function rename($from,$to){
		if(!ftp_site($this->connection, "RNFR $from")) { // from name
			$this->error = "Could not rename ($from) to $to"; 
			return false;
		}
		if(!ftp_site($this->connection, "RNTO $to")) { // to name
			$this->error = "Could not rename ($from) to $to"; 
			return false;
		}
		return true;
	}

	// ----------------------------------------------------------------
	function ChDir($path){

		if(!ftp_chdir($this->connection, $path)) { // go to destination dir
			$this->error = "Could not change ftp directory to $path";
			return false;
		}
		return true;

	}


	// ----------------------------------------------------------------
	// create directory through FTP connection
	function Mkdir($path, $newDir, $permissions = null) {
 
		if($path && !ftp_chdir($this->connection, $path)) { // go to destination dir
			$this->error = "Could not change directory to $path";
			return false;
		}
		if(!ftp_mkdir($this->connection,$newDir)) { // create directory
			$this->error = "Could not create new directory ($newDir) in $path"; 
			return false;
		}
		if($permissions){
			$this->Chmod($newDir,$permissions);
		}

		return true;
	}

	// ----------------------------------------------------------------
	function Chmod($path = null,$num = 755){
		
		if(getenv("windir") != ""){
			return true;	
		}

		$num = intval($num);
		
		if($path){
			if(!ftp_site($this->connection, "CHMOD $num $path")) { // change attributes
				$this->error = "Could not chmod ($path) to $num"; 
				return false;
			}
			return true;
		}
		else{
			$this->error = "Could not chmod ($path) to $num"; 
			return false;
		}
	}

}
?>