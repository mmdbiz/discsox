<?php

class Downloads{
	
	function __construct(){
	}

	// ----------------------------------------------------
	function makeDownloadLinks($orderNumber,$email = null){
		
		global $_CART;
		global $_CF;
		global $_Common;
		global $_DB;

		$downloadHost = $_CF['downloads']['download_url'];
		$downloadPath = $_CF['downloads']['download_filepath'];
		$downloadLinks = array();
		$downloadList = array();
		
		foreach($_CART as $i=>$fields){
			if(isset($fields['is_downloadable']) && $fields['is_downloadable'] == "true" && trim($fields['download_filename']) != ""){
				
				$id = trim($fields['sku']);
				$name = trim($fields['name']);
				$filename = trim($fields['download_filename']);
				
				if(file_exists("$downloadPath/$filename")){
					// make key
					$time = time();
					$key = md5($time . $email . $id);
					$sql = "INSERT INTO downloads (`order_number`,`email`,`filename`,`key`) 
											VALUES('$orderNumber','$email','$filename','$key')";
					$_DB->execute($sql);
					$downloadLinks[] = "<a href=\"$downloadHost/download.php?id=$key\">$name</a>";
				}
			}
		}
		return $downloadLinks;
	}


	// ----------------------------------------------------
	function getFile(){
		
		global $_DB;
		global $error;

		$key = trim($_REQUEST['id']);

		$sql = "SELECT filename,complete FROM downloads WHERE `key` = '$key' LIMIT 1";
		$data = $_DB->getRecords($sql);

		if(isset($data[0]['filename'])){
			if($data[0]['complete'] == "true"){
				$error = "This download has already been completed.";
			}
			else{
				$downloadFile = strtolower($data[0]['filename']);
				if($this->do_download($downloadFile)){
					// update complete in database
					$sql = "UPDATE downloads SET complete = 'true' WHERE `key` = '$key'";
					$_DB->execute($sql);
				}
				else{
					if($error == "Broken connection"){
						// stop because the connection was broken before it completed
						die();	
					}
				}
			}
		}
		else{
			$error = "You are unathorized to download this file.";	
		}
	}

	// --------------------------------------------------------
	function do_download($download) {

		global $_CF;
		global $error;
		
		$downloadPath = $_CF['downloads']['download_filepath'];
		$filePath = "$downloadPath/$download";

		// content-types:
		//  .rm
		// audio/x-pn-realaudio
		// application/vnd.rn-realmedia 
		// audio/vnd.rn-realaudio 
		// audio/x-pn-realaudio 
		// audio/x-realaudio 
		// audio/x-pm-realaudio-plugin
		
		//  .wmf
		// image/x-wmf
		// image/x-wmf
		// application/wmf 
		// application/x-wmf

		if(file_exists($filePath)){
			
			// the user can stop the output, but not the script.
			ignore_user_abort(true);

			// send the requested file to the user..
			header("HTTP/1.1 200 OK");
			header("Status: 200 OK");
			header("Content-type: application/zip");
			header("Content-Disposition: attachment; filename=$download");
			header("Content-length:".(string)(filesize($filePath)));
			header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
			header("Pragma: no-cache");
			header("Expires: 0" . "\n");
		    
			$file = fopen($filePath,"rb");    
			while(!feof($file)){ 
				// send 8k at a time
				print(fread($file, 1024*8));
				flush();
				//check the connection to see if it has ended... 
				if(connection_status()!=0){
					fclose($file);
					$error = "Broken connection";
					return false;
				} 
			} 
			fclose($file);
		}
		else{
			$error = "This download file is missing. Please contact the site administrator to inform them and to get your download.";
			return false;	
		}
		
	return true;
	}	
	
	
}

?>