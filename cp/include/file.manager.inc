<?php
//VersionInfo:Version[3.0.1]

	// backup directories
	// drop & drag backup files

class File_Manager{

    var $debug = false;
    var $rootDir = NULL;
    var $validExtensions = array("htm"	=> 1,
								 "html"	=> 1,
								 "xml"	=> 1,
								 "css"	=> 1,
								 "txt"	=> 1,
								 "php"	=> 1,
								 "gif"	=> 1,
								 "jpg"	=> 1,
								 "png"	=> 1,
								 "csv"	=> 1,
								 "js"	=> 1,
								 "inc"	=> 1);
								 
    var $imageTypes = array("gif"	=> 1,
							"jpg"	=> 1,
							"jpeg"	=> 1,
							"png"	=> 1,
							"swf"	=> 1,);
							
	var $dirIndex = array();
	var $fileIndex = array();
	var $lookInSubdirectories = true;
	var $cwd = NULL;
    
    // -------------------------------------------------------------------
    function __construct(){

		// backup to the root, not in here.
		$cwd = getcwd();
		if($cwd == ""){
			$this->rootDir = "../";
			chdir("../");
		}
		else{
			chdir("../");
			$this->rootDir = getcwd();
		}
		$this->rootDir = str_replace("\\","/",$this->rootDir);
		chdir("./cp");
	}
	
	// -------------------------------------------------------------------
	function listDirectories($dir = NULL){
		
		global $_Common;
		
		if(!$dir){
			$dir =  $this->rootDir;	
		}
		
		//$_Common->debugPrint($dir); 
		
		if(!is_dir($dir)){
			die($dir . " is not a valid directory!");
			return;
		}
		
		@chmod($dir,0755);
		
		if(!($handle = opendir($dir))){
			die("Access denied to $dir");
			return;
		}
		$dir_count=0;
		while($name = readdir($handle)){
			if($name == ".." || $name == "."){
				continue;
			}
			if(is_dir("$dir/$name") && $name != "cp"){
				$this->dirIndex[$dir_count++] = $name;
			}
			elseif(is_file("$dir/$name") && $_Common->getFileExtension($name,$this->validExtensions)){
				$icon = $this->getIcon("$dir/$name");
				
				$size = $this->convertFileSize(filesize("$dir/$name"));
				$perms = $this->getFilePermissions(fileperms("$dir/$name"));
				
				$this->fileIndex[$name]['icon'] = $icon;
				$this->fileIndex[$name]['size'] = $size;
				$this->fileIndex[$name]['perms'] = $perms;
			}
		}
		closedir($handle);
	}

	//---------------------------------------------------------------
	function getIcon($file){
		global $_Common;
		
		$icon = NULL;
		$pathinfo = pathinfo($file);
		
		//$_Common->debugPrint($pathinfo);
		
		if(!empty($pathinfo['extension'])){
			switch($pathinfo['extension']){
				case "gif":
				case "jpg":
				case "jpeg":
				case "png":
					$icon = "image";
					break;
				case "htm":
				case "html":
					$icon = "html";
					break;
				case "txt":
					$icon = "txt";
					break;
				case "css":
					$icon = "css";
					break;
				case "csv":
					$icon = "csv";
					break;
				case "js":
					$icon = "js";
					break;
				case "php":
				case "inc":
					$icon = "php";
					break;
				case "xml":
					$icon = "xml";
					break;
			}
		}
	
		if($icon){
			if(file_exists("icons/$icon.gif")){
				$icon = "<img src='icons/$icon.gif' width='20' height='20'>";
			}
		}	
		
	return $icon;
	}
	
	//---------------------------------------------------------------
	function convertFileSize($size){
		if(0 > $size){
			return $size;
		}
		$filesizename = array(" Bytes", " KB", " MB", " GB", " TB", " PB", " EB", " ZB", " YB");
		$size = @round($size/pow(1024, ($i = floor(log($size, 1024)))), 2);
		if(!empty($filesizename[$i])){
			return  $size . $filesizename[$i];
		}
	}
	
	//---------------------------------------------------------------
	function getFilePermissions($filePerms){
		
		$perms = "";
		// owner
		$perms .= (($filePerms & 0x0100) ? 'r' : '&minus;') .
				  (($filePerms & 0x0080) ? 'w' : '&minus;') .
				  (($filePerms & 0x0040) ? (($filePerms & 0x0800) ? 's' : 'x' ) :
										   (($filePerms & 0x0800) ? 'S' : '&minus;'));

		// group
		$perms .= (($filePerms & 0x0020) ? 'r' : '&minus;') .
				  (($filePerms & 0x0010) ? 'w' : '&minus;') .
				  (($filePerms & 0x0008) ? (($filePerms & 0x0400) ? 's' : 'x' ) :
										   (($filePerms & 0x0400) ? 'S' : '&minus;'));

		// world
		$perms .= (($filePerms & 0x0004) ? 'r' : '&minus;') .
				  (($filePerms & 0x0002) ? 'w' : '&minus;') .
				  (($filePerms & 0x0001) ? (($filePerms & 0x0200) ? 't' : 'x' ) :
										   (($filePerms & 0x0200) ? 'T' : '&minus;'));
	return $perms;
	}
	
	//---------------------------------------------------------------
	function chmodnum($strPerms) {
		
		global $_Common;
		
		$strPerms = str_replace("&minus;","-",$strPerms);
		$nums = array('-'=>'0','r'=>'4','w'=>'2','x'=>'1');
		$chmod = 0;
		
		//$_Common->debugPrint($strPerms);
		
		for($i=0;$i<strlen($strPerms);$i+=3){
			$block = substr($strPerms,$i,3);
			$num = 0;
			for($j=0;$j<3;$j++){
				if(isset($nums[substr($block,$j,1)])){
					$num += $nums[substr($block,$j,1)];
				}
			}
			$chmod .= $num;
		}
		return $chmod;
	}


	// --------------------------------------------------------------
	function doUploads($dir = ""){

		global $_Common;
		global $uploadStatus;
		global $_CF;
		global $requestDir;
		
//$_Common->debugPrint($_FILES['upfile']);
//exit;

//$_Common->debugPrint($requestDir);
//exit;

		// try to set permissions
		$dPerms = $this->getFilePermissions(fileperms($dir));
		$dChmod = $this->chmodnum($dPerms);
		@chmod($dir,0777);

		if(count($_FILES) > 0){
			// include FTP in case we need it.
			include_once("ftp.inc");
			$ftp = new Ftp();
			$ftpRoot = $_CF['ftp']['document_root'];
			$ftp->ChDir($ftpRoot);
		}

        foreach($_FILES as $fldName=>$fData){

			$fCount = count($fData);
			for($i=0;$i<$fCount;$i++){
				if(isset($fData['error'][$i]) && $fData['error'][$i] > 0){
					continue;
				}
				if(isset($fData['tmp_name'][$i]) && $fData['tmp_name'][$i] == "none"){
					continue;
				}
				if(!isset($fData['name'][$i])){
					continue;	
				}
				$fName = $fData['name'][$i];
				if($_Common->getFileExtension($fName,$this->validExtensions)){
					if(@move_uploaded_file($fData['tmp_name'][$i], "$dir/$fName")){
						$uploadStatus[$fName] = "Success";
					}
					else{
						if(!is_null($requestDir)){
							$path = "$requestDir/$fName";
						}
						else{
							$path = $fName;
						}
						if(!$ftp->Upload($fData['tmp_name'][$i],$path)){
							$ftp->Close();
							$uploadStatus[$fName] = $ftp->error;
						}
					}
				}
				else{
					$uploadStatus[$fName] = "Failed: Invalid file name";	
				}
			}
        }
        
        if(intval($dChmod) < 600){
			$dChmod = "0755";	
		}
        
        // reset back to original permissions
        @chmod($dir,$dChmod);

        if($ftp){
	        $ftp->Close();
	    }
       
		//$_Common->debugPrint($uploadStatus);
		//exit;
	}

	// --------------------------------------------------------------
	function update($file,$data){
		
		global $_CF;
		
		// try to set permissions
		$fPerms = $this->getFilePermissions(fileperms($file));
		$fChmod = $this->chmodnum($fPerms);
		@chmod($file,0777);
		
		if(!($H = @fopen($file, "w"))){
			$fName = basename($file);
			
			include_once("ftp.inc");
			$ftp = new Ftp();
			$ftpRoot = $_CF['ftp']['document_root'];
			$ftp->ChDir($ftpRoot);
			
			$requestDir = null;
			if(isset($_REQUEST['dir']) && trim($_REQUEST['dir']) != ""){
				$requestDir = trim($_REQUEST['dir']);
			}
			$ftp->updateFile("./$requestDir/$fName",$data);
			$ftp->Close();
		}
		else{
			flock($H,2);
			fputs($H,$data);
			flock($H,3);
			fclose($H);
		}
		
		if(intval($fChmod) <= 644){
			$fChmod = "0755";	
		}

		
		// reset to original permissions
		@chmod($file,0644);
	}


	// --------------------------------------------------------------
	function delete(){

		$filesDeleted = 0;
		$dirsDeleted = 0;

		if(isset($_REQUEST['sel'])){
			foreach($_REQUEST['sel'] as $i=>$file){
				if(is_file($file)){
					unlink($file);
					$filesDeleted++;
				}
				elseif(is_dir($file)){
					$this->rmdirr($file);
					$dirsDeleted++;
				}
			}
		}
		
	return array($filesDeleted,$dirsDeleted);	
	}

	// --------------------------------------------------------------
	function rmdirr($dir) {
		if($objs = glob($dir."/*")){
			foreach($objs as $obj) {
				is_dir($obj) ? $this->rmdirr($obj) : unlink($obj);
			}
		}
		rmdir($dir);
	} 


    // -------------------------------------------------------------------
    function image_createThumb($src,$dest,$maxWidth,$maxHeight,$quality=100) {

        if(file_exists($src) && isset($dest)) {

            if(!extension_loaded('gd')) {
                $this->showError("The PHP GD2 Image library must be loaded to use the thumbnail functions.");
            }

            // path info
            $destInfo  = pathInfo($dest);

            // image src size
            $srcSize  = getImageSize($src);

            // image dest size $destSize[0] = width, $destSize[1] = height
            $srcRatio  = $srcSize[0]/$srcSize[1]; // width/height ratio
            $destRatio = $maxWidth/$maxHeight;
            if($destRatio > $srcRatio) {
                $destSize[1] = $maxHeight;
                $destSize[0] = $maxHeight*$srcRatio;
            }
            else{
                $destSize[0] = $maxWidth;
                $destSize[1] = $maxWidth/$srcRatio;
            }

            // true color image
            $destImage = imageCreateTrueColor($destSize[0],$destSize[1]);

            // src image
            switch ($srcSize[2]) {
                case 1: //GIF
                if(function_exists('imageCreateFromGif')){
                    $srcImage = imageCreateFromGif($src);
                }
                else{
                    $errMessage = "Support for converting GIF files to thumbnails is not available</p>
								   <p>Please uncheck the \"Automatically Create Thumbnail\" to upload your
								   GIF thumbnail images or use jpg or png fullsize images.";
                    die($errMessage);
                }
                break;

                case 2: //JPEG
                $srcImage = imageCreateFromJpeg($src);
                break;

                case 3: //PNG
                $srcImage = imageCreateFromPng($src);
                break;

                default:
                return false;
                break;
            }

			$gdInfo = gd_info();
			if($gdInfo['GIF Create Support'] != 1 && $destInfo['extension'] == "gif"){
				imageCopyResized($destImage, $srcImage, 0, 0, 0, 0,$destSize[0],$destSize[1],$srcSize[0],$srcSize[1]);
			}  
			else{
	            // resampling
		        imageCopyResampled($destImage, $srcImage, 0, 0, 0, 0,$destSize[0],$destSize[1],$srcSize[0],$srcSize[1]);
			}

            // generating image
            switch ($srcSize[2]) {
                case 1:
                case 2:
                imageJpeg($destImage,$dest,$quality);
                break;
                case 3:
                imagePng($destImage,$dest);
                break;
            }
            imagedestroy($destImage);
            imagedestroy($srcImage);
            
            return true;
        }
        else{
            return false;
        }
    }


	// --------------------------------------------------------
	function do_download($filePath) {

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
			$download = basename($filePath);
			// send the requested file to the user..
			header("Content-type: application/octet-stream");
			header("Content-Disposition: attachment; filename=$download");
			header("Content-length:".(string)(filesize($filePath)));
			header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
			header("Pragma: no-cache");
			header("Expires: 0" . "\n");

			// Alternative headers for downloading in HTTPS
//			header("Pragma: public"); 
//			header("Expires: 0"); 
//			header("Cache-Control: must-revalidate, post-check=0, pre-check=0"); 
//			header("Cache-Control: private",false); 
//			header ( "Content-Type: application/octet-stream" ); 
//			header("Content-Disposition: attachment; filename=\"".$download."\";"); 
//			header("Content-Transfer-Encoding:­ binary"); 
//			header("Content-Length: ".filesize($filePath)); 

		    
			$file = fopen($filePath,"rb");    
			while(!feof($file)){ 
				// send 8k at a time
				print(fread($file, 1024*8));
				flush();
				//check the connection to see if it has ended... 
				if(connection_status()!=0){
					fclose($file);
					return false;
				} 
			} 
			fclose($file);
		}
		
	return true;
	}	


}

?>