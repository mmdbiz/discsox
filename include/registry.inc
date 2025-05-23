<?php

//VersionInfo:Version[3.0.1]

class Registry{

    var $debug = false;
    var $registeredClasses = array();
    var $includePaths = array();

    function __construct($includePaths = null){
		
		global $_CF;
		global $_Common;
		
		if(!$includePaths){
			$this->includePaths = explode(PATH_SEPARATOR, get_include_path());
		}
		else{
			$this->includePaths = explode(PATH_SEPARATOR, $includePaths);
		}

		if($this->debug){
			$_Common->debugPrint($this->includePaths,"Registry Include Paths");
		}
	}

	//----------------------------------------------------------------
	function getExtensions(){
		global $_DB;
		$sql = "SELECT * FROM extensions";
		$data = $_DB->getRecords($sql);
		if(count($data) > 0){
			foreach($data as $i=>$fields){
				$this->registeredClasses[$fields['class_to_extend']] = $fields['extended_class_name'];	
			}	
		}
	}


    //----------------------------------------------------------------
    // Function name
    //     loadClass
    // General Description
    //     Loads a class library that may have a call in it
    //     that extends the default functions.
    // Inputs
    //      $class - class name to load.
    //		$func - paramater to feed to class constructor.
    // Outputs
    //     Either the class object or null
    // Notes
    //		To extend a class the class must be in the extensions section
    //		in the config and the class name in the file must be "x_ParentClassName"
    
    function loadClass($class,$func = NULL){

		global $_CF;
		global $_Common;
		global $_DB;
		
		if($_DB){
			$this->getExtensions();
		}

		$class = trim(strtolower($class));
		//get the file name to load for the class.
		$libName = "$class.inc";
        $ClassName = ucwords($class);

		// Check to see if it's in the include path and load
		$fPath = $this->file_exists_incpath($libName);

        if($fPath){
			if(!class_exists($ClassName)){
				//@include_once($fPath); //marcello
				//echo ("classname: " . $ClassName . " fPath:  " .$fPath  ); //marcello
				include_once($fPath);
//echo "got here registry.inc..."; //marcello
				if($this->debug){
					$_Common->debugPrint("Parent class loaded: $libName");
				}
			}
			else{
				if($this->debug){
					$_Common->debugPrint("Parent class already loaded: $libName");
				}
			}
        }
        else{	
			die("Registry Error! Could not load $class class library");
			exit;	
		}

		// Check for extended classes of the payment class automatically.
		if(strtolower($class) == "payment" && $func != ""){
			$exClass = strtolower($class) . "." . strtolower($func) . ".inc";
			$exPath = $this->file_exists_incpath($exClass);
			if($exPath){
				$this->registeredClasses['payment'] = $exClass;
			}
		}
		
		// Check for extensions and try to load
		if(isset($this->registeredClasses[$libName])){
			
			//get the full file name for the class that extends the base.
			$ClassLib = $this->registeredClasses[$libName];
			if($this->debug){
                $_Common->debugPrint("\t<b>\"$class\" has a registered extension class to load: $ClassLib</b>");
            }
            $classLoaded = false;
            $exPath = $this->file_exists_incpath($ClassLib);
            if($ClassLib != "" && $exPath){
				$classLoaded = include_once($exPath);
				if($this->debug){
					$_Common->debugPrint("\tExtension class included: $ClassLib, $classLoaded");
				}
			}

			$class = str_replace('.','_',$class);
			$exClass = "x_" . ucwords($class);

            //check that the class realy exists before trying to create it.
            if($classLoaded && class_exists($exClass)){
				$ClassName = "x_" . ucwords($class);
            }
            if(!$classLoaded){
				if($this->debug){
					$_Common->debugPrint("\tCould not load extension class: $ClassLib. The class file was not found...");
				}
            }
            elseif(!class_exists($exClass)){
				//unset(
				if($this->debug){
					$_Common->debugPrint("\tCould not load extension class: $ClassLib. The class name ($exClass) was not found...");
				}
			}
		}

		//Replace any "." with "_" so that the class will load
		$ClassName = preg_replace("/\./i", '_', $ClassName);

		//now load the class
		if($func){
			$ClassObject = new $ClassName($func);
		}
		else{
			$ClassObject = new $ClassName();
		}

		return $ClassObject;
    }

	// Checks to see if the class file resides in one of the include dirs.
	function file_exists_incpath ($file){
	
		foreach($this->includePaths as $i=>$path) {
			// Formulate the absolute path
			$fullpath = $path . DIRECTORY_SEPARATOR . $file;
			// Check it
			if(file_exists($fullpath)) {
				return $fullpath;
			}
		}
		return false;
	}

}		

?>