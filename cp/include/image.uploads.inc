<?php
//VersionInfo:Version[3.0.1]

class Image_uploads {

    var $debug = false;
    var $ftp = null;
    
    // --------------------------------------------------------------
    function __construct() {

	}

	// --------------------------------------------------------------
    function uploadImages(&$images,$categoryImage = false){

        global $_CF;
		global $_Common;
		
        // Example:
        //[name] => thumbnail.gif
        //[type] => image/gif
        //[tmp_name] => C:/PHP/uploadtemp\php1B.tmp
        //[error] => 0
        //[size] => 5064

		//$_Common->debugPrint($_FILES);
		//exit;
		
		if(count($_FILES) > 0){
			// include FTP in case we need it.
			include_once("ftp.inc");
			$ftp = new Ftp();
			$ftpRoot = $_CF['ftp']['document_root'];
			$ftp->ChDir($ftpRoot);
			$this->ftp = $ftp;
			$ftp->Chmod($_CF['images']['fullsize_images_directory'],777);
			$ftp->Chmod($_CF['images']['thumbnail_images_directory'],777);
		}

		$curDir = getcwd();
		// move to the document root if need be:
		if(basename(getcwd()) == "cp"){
			@chdir ('../');
		}


		$createThumbnail = false;
		if(!empty($_REQUEST['create_thumbnail']) && $_REQUEST['create_thumbnail'] == "true"){
			$createThumbnail = true;
		}

		
        foreach($_FILES as $fldName=>$imgData){

            if(isset($imgData['error']) && $imgData['error'] > 0){
                continue;
            }
            if(isset($imgData['tmp_name']) && $imgData['tmp_name'] == "none"){
                continue;
            }

            $fldName = preg_replace("|file-|","",$fldName);

            $imageDir = $_CF['images']['fullsize_images_directory'];
            $thumbDir = $_CF['images']['thumbnail_images_directory'];

            @chmod($imageDir,0777);
            @chmod($thumbDir,0777);

            $type = $imgData['type'];
            $imgName = $imgData['name'];
            
            if($fldName == "thumbnail_image"){
				$imageDir = $thumbDir;
			}

			if(@move_uploaded_file($imgData['tmp_name'], "$imageDir/$imgName")){
				@chmod("$imageDir/$imgName",0755);
			}
			else{
				$fPath = $_CF['images']['fullsize_images_directory'] . "/" . $imgName;
				if($fldName == "thumbnail_image"){
					$fPath = $_CF['images']['thumbnail_images_directory'] . "/" . $imgName;
				}
				if(!$ftp->Upload($imgData['tmp_name'],$fPath,true)){
					$ftp->Close();
					die($ftp->error);
				}
				else{
					$ftp->Chmod($fPath,755);	
				}
			}

            if($type != "image/jpeg" && $type != "image/pjpeg" && $type != "image/gif" && $type != "image/x-png"){
				unlink("$imageDir/$imgName");
                $this->showError("Invalid image type. Only jpg, png, and gif images are allowed to be uploaded");
            }

			$pathinfo = pathinfo("$imageDir/$imgName");
			$ext = NULL;
			if(!empty($pathinfo['extension'])){
				$ext = strtolower($pathinfo['extension']);
			}

            if($createThumbnail){

                    // if not a category image and not the fullsize
                    // image, skip the create thumnnail

                if(!$categoryImage && !stristr($fldName,"fullsize_image")){
                    $images[$fldName] = $imgData['name'];
                    continue;
                }

                $destImage = $imgName;

                if($ext == "jpg" || $ext == "jpeg" || $ext == "png"){

					if($categoryImage || stristr($fldName,"fullsize_image")){
						$img = $imgData['name'];
					}

                    $maxHeight = intval($_CF['images']['product_thumbnail_max_height']);
                    $maxWidth = intval($_CF['images']['product_thumbnail_max_width']);

                    if($maxWidth){
						$maxWidth = intval($maxWidth);
					}else{
						$maxWidth = 100;
					}

					if($maxHeight){
						$maxHeight = intval($maxHeight);
					}else{
						$maxHeight = 150;
					}

                    $this->image_createThumb("$imageDir/$imgName","$thumbDir/$destImage",$maxWidth,$maxHeight);
                    @chmod("$thumbDir/$destImage",0755);
					if($categoryImage){
	                    $images['category_thumbnail'] = $destImage;
	                }
	                else{
	                    $images['thumbnail_image'] = $destImage;
					}
                }
                else{
                    $img = $imgData['name'];
                }
            }
            else{
                $img = $imgData['name'];
            }
            
			if($categoryImage){
				$fldName = "category_thumbnail";
			}

            //print $fldName;
			$images[$fldName] = $img;
        }
        
        if($ftp){
	        $ftp->Close();
	    }
	    @chdir($curDir);
    }
    
    // -------------------------------------------------------------------
    function image_createThumb($src,$dest,$maxWidth,$maxHeight,$quality=100) {

		global $_CF,$_Common;

		//$_Common->debugPrint($src,$dest);

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

            // true color image, with anti-aliasing
            $destImage = imageCreateTrueColor($destSize[0],$destSize[1]);
            //@imageAntiAlias($destImage,true);

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
                    $this->showError($errMessage);
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
			
			$thumbDir = $_CF['images']['thumbnail_images_directory'];
			if(substr($thumbDir,0,1) == '/'){
				$thumbDir = substr($thumbDir,1,strlen($thumbDir));	
			}
			
			$dest = "$thumbDir/" . basename($dest);

			//print $dest;

            // generating image
            switch ($srcSize[2]) {
                case 1:
                case 2:
                if(!@imageJpeg($destImage,$dest,$quality)){
					// put file into temp file and ftp upload
					ob_start();
					$temp = tmpfile();
					imageJpeg($destImage,'',$quality);
					fwrite($temp,ob_get_contents());
					ob_end_clean();
					fseek($temp,0);
					if(!$this->ftp->UploadFile($temp,$dest,true)){
						$this->ftp->Close();
						die($this->ftp->error);
					}
					else{
						$this->ftp->Chmod($dest,755);	
					}
					fclose($temp);
				}
                break;
                case 3:
                if(!@imagePng($destImage,$dest)){
					// put file into temp file and ftp upload
					ob_start();
					$temp = tmpfile();
					imagePng($destImage);
					fwrite($temp,ob_get_contents());
					ob_end_clean();
					fseek($temp,0);
					if(!$this->ftp->UploadFile($temp,$dest,true)){
						$this->ftp->Close();
						die($this->ftp->error);
					}
					else{
						$this->ftp->Chmod($dest,755);	
					}
					fclose($temp);
				}
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


	// --------------------------------------------------------------
    ///	Displays internal errors.
    function showError($error){
        print "<html><head><title>$error</title>";
        print "<link rel=\"stylesheet\" type=\"text/css\" href=\"stylesheets/admin.800.css\">";
        print "</head><body>";
        print "<div align=center>\n";
        print "<h4><br>Error</h4>\n";
        print "<p>$error</p>\n";
        print "<form><input type=button value=\"Go Back\" onClick=\"javascript:history.go(-1);\"></form>\n";
        print "</div>\n";
        print "</body></html>";
        exit;
    }
}
?>