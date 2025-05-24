<?php

// -------------------------------------------------------------------
// General encryption class
// modified MD5 is used for built-in functions
// PGP or GNUPG is also available below

class Crypt{

	public bool $debug = false;
	public $key;
	public $rand;

	function __construct($key = "", $rand = null){
		global $_CF;
		if($_CF['encryption']['use_default_encryption']){
			if($key == ""){
				$key = trim($_CF['encryption']['enter_default_public_key']);
				if($key == ""){
					die("Encryption Error: No public key was set in the configuration file?");
					exit;
				}
			}
			$this->key = $key;
			if(!isset($rand)){
				// srand() is deprecated and not needed since PHP 4.2.0
				$rand = rand(1, 32767);
				$rand = pack('i*', $rand);
			}
			$this->rand = $rand;			
		}
	}

    function encrypt($data){
		global $_CF,$_CR;
		
		if($_CF['encryption']['use_pgp']){
			return $this->pgp_encrypt_message($data);
		}
		else{
			$data = "abcdefghijklmno$_CR$data"; 
			$data = unpack('C*', $data);
			$ans = array();
			$ans1 = array();
			$eblock = 1;
			$e_block = $this->newKey($this->rand);
			$data_size = count($data);
			for($i = 0; $i < $data_size; $i++){
				$mod = $i % 16;
				if(($mod == 0)&&($i > 15)){
					$tmparr = array($ans[$i - 15], $ans[$i - 14], $ans[$i - 13], $ans[$i - 12], $ans[$i - 11], $ans[$i - 10], $ans[$i - 9], $ans[$i - 8], $ans[$i - 7], $ans[$i - 6], $ans[$i - 5], $ans[$i - 4], $ans[$i - 3], $ans[$i - 2], $ans[$i - 1], $ans[$i]);
					$tmparr = $this->array2pack($tmparr);
					$tmparr = implode('', $tmparr);
					$e_block = $this->newKey($tmparr);
				}
				$mod++;$i++;
				$ans[$i] = $e_block[$mod] ^ $data[$i];
				$ans1[$i] = pack('C*', $ans[$i]);
				$i--;$mod--;
			}
			return $this->formatOutput(implode('', $ans1));
		}
    }

    function binmd5($string){
        return pack('H*', strtoupper(md5(pack('H*', preg_replace('|00$|', '', bin2hex($string))))));
    }

    function array2pack($array){
        $pack = array();
        foreach($array as $val){$pack[] = pack('C*', $val);}
        return $pack;
    }

	function newKey($round){
        $digest = $this->binmd5($this->key.$round);
        return unpack('C*', $digest);
    }


	function formatOutput($ciphertext){
		global $_CR;
		$ciphertext = strtoupper(bin2hex($ciphertext));
		$count = 0;
		$crypttext = "";
		while($count <= strlen($ciphertext)){
			$crypttext .=  substr($ciphertext,$count,50) . $_CR;
			$count += 50;
		}
		return $crypttext;
	}
    
	function encryptOrderFile($tempFile, $tempData){
		global $_CF,$_CR;
		
		if($_CF['encryption']['use_pgp']){
			$crypttext = $this->pgp_encrypt_message($tempData);
		}
		else{
			$ciphertext = $this->encrypt($tempData);
			$crypttext = $this->formatOutput($ciphertext);
		}
		$tFile = fopen($tempFile,'w');
		if(!$tFile){
			$tempFile = basename($tempFile);
			die("Could not write to temp file: $tempFile\n");
			exit;
		}
		fputs($tFile,$crypttext);
		fclose($tFile);
	}
	    
	function pgp_encrypt_message($plainText) {

		global $_CF,$_Common;

		// Define PGP Variables
		$pubKey = $_CF['encryption']['public_key_id'];
		// pgp_path defines the location of the PGP or GNUPG program on your unix server
		$pgp_path = $_CF['encryption']['pgp_path'] . "/" . $_CF['encryption']['pgp_executable_name'];
		// pgp_config_files is the location of pubring.pgp, randseed.bin and config.txt
		$pgp_config_files = $_CF['encryption']['pgp_path'];
		$pgp_temp_file_path = $_CF['encryption']['pgp_temp_file_path'];
		// Your local time zone
		$timeZone = $_CF['encryption']['pgp_time_zone'];
		
		// Temp files
		$tempName = substr(session_id(),0,8);
		$input_file = $pgp_temp_file_path . "/" . "$tempName.pgp";
		$output_file = $pgp_temp_file_path . "/" . "$tempName.asc";

		// Set the environment variables
		putenv("HOME=$pgp_config_files");
		putenv("PGPPATH=$pgp_config_files");
		putenv("GPGPATH=$pgp_config_files");
		putenv("TZ=$timeZone");
		putenv("TMP=$pgp_temp_file_path");
		putenv("GNUPGHOME=$pgp_temp_file_path");

		$pgp_options = "--always-trust --no-secmem-warning --no-permission-warning -a -r $pubKey -o $output_file -e $input_file";

		$encrypted_text = null;

		// Write a plain text temp file
		if(!($tempFile = fopen($input_file, "w"))){
			$input_file = basename($input_file);
			die("PGP Error: $input_file could not be created");
			exit;
		}

		flock($tempFile,2);
		fputs($tempFile,$plainText);
		flock($tempFile,3);
		fclose($tempFile);

		$pgp_command =  "$pgp_path $pgp_options 2>&1";
		
		// Send the command to the program
		$results = array();
		exec($pgp_command,$results,$s);

		$encrypted_text = file_get_contents($output_file);

		if($this->debug){
			$_Common->debugPrint($pgp_temp_file_path,"TEMP PATH");
			$_Common->debugPrint($pgp_command,"PGP COMMAND:");
			$_Common->debugPrint($results,"PGP ERRORS:");
			$_Common->debugPrint($encrypted_text,"PGP OUTPUT:");
		}

		if(trim($encrypted_text) == "") {
			$pgp_error = "Script Error: PGP produced no output when processing this message. \n";
			$pgp_error .= "The most likely cause for this is that the PGP key you submitted \n";
			$pgp_error .= "is invalid or you did not specify the correct location of pubring.pgp \n";
			$pgp_error .= "in the configuration file.\n";
			die($pgp_error);
			exit;
		}

		unlink($input_file);
		unlink($output_file);

		return($encrypted_text);
	}

}


?>