

function selectAll(formObj, select){
    for(var i=0;i < formObj.length;i++){
        fldObj = formObj.elements[i];
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
function checkSelected(formObj,type,recordType){
    var found = false;
    for(var i=0;i < formObj.length;i++){
        fldObj = formObj.elements[i];
        if(fldObj.type == 'checkbox'){
            if(fldObj.checked){
                found = true;
            }
        }
    }
    if(!found){
        alert("No records have been selected to " + type + "?");
        return false;
    }

    if(type == "delete"){
        return confirmDelete(recordType);
    }

return true;
}

function confirmDelete(type){
    var result = confirm('Are you sure you want to delete the selected ' + type + '(s) and all of their corresponding records?');
    return result;
}


function strip(val) {
	// Strips non digits from a str
	val = "" + val;
	if (!val)
		return "";
	var result = "";
	for (i=0; i < val.length; i++) {
		character = val.charAt(i);
		if ("0123456789".indexOf(character) != -1)
		result += character;
	}
	return result;
}


function commify (nbr) { 
	return nbr.toString().split('').reverse().join('').replace(/(?=\d*\.?)(\d{3})/g,'$1,').split('').reverse().join('').replace(/^[\,]/,'');
}








