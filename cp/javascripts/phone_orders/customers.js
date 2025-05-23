

/*
This class does the auto-complete lookup selections for customer fields
in the phone.orders.php form.
*/

var customer = new function(){

	// initialize auto-complete fields
	this.resultFld = 'customerLookupContainer';

	this.fields = new Array("records",
							"billaddress_lastname",
							"billaddress_firstname",
							"cid",
							"billaddress_companyname",
							"billaddress_addr1",
							"billaddress_addr2",
							"billaddress_city",
							"billaddress_state",
							"billaddress_county",
							"billaddress_postalcode",
							"billaddress_country",
							"billaddress_areacode",
							"billaddress_phone",
							"billaddress_email",
							"email_list",
							"is_taxable");

	// Setup the XHR DataSource for all cart queries
	this.dataSource = new YAHOO.widget.DS_XHR("../request.php", this.fields);
	// configure the response type to be JSON (default)
	this.dataSource.responseType = YAHOO.widget.DS_XHR.TYPE_JSON;
	this.dataSource.maxCacheEntries = 25;
    this.dataSource.scriptQueryAppend = "table=customers&fields=billaddress_lastname,*";

	this.doLookup = function(){

		// set the AC to the new field
		var myAutoComp = new YAHOO.widget.AutoComplete('customers[billaddress_lastname]',customer.resultFld,customer.dataSource);
		// AC options
		myAutoComp.queryDelay = 1;
		myAutoComp.minQueryLength = 2;
		myAutoComp.maxResultsDisplayed = 25;
		myAutoComp.autoHighlight = false;
		myAutoComp.useShadow = true;

		var customerData = [];
		var cid = null;

		myAutoComp.formatResult = function(aResultItem, sQuery) {
            //alert(aResultItem);
			var lastName = aResultItem[0];
			var firstName = aResultItem[1];
			var city = aResultItem[6];
			return lastName + ', ' + firstName + ', ' + city;
		}

		var respHandler = function(type, args, me) {

            //alert(args[2]);
			// get the fields into an array so we can populate the form fields
			for(var i=0;i<=args[2].length;i++) {
				j = i + 1;
				customerData[customer.fields[j]] = args[2][i];
			}
			cid = customerData['cid'];

			for (var i in customerData) {
				if(i != 'toJSONString'){
					//alert('key is: ' + i + ', value is: ' + customerData[i]);
					var fldName = 'customers[' + i + ']';
					var isFld = dom.get(fldName);
					if(isFld != null){
						if(isFld.type == "text" || isFld.type == "hidden"){
							document.getElementById(fldName).value = customerData[i];
						}
						else{
							if(isFld.type == "select-one" || isFld.type == "radio"){
								customer.pickValue(isFld,customerData[i]);
							}
						}
					}
				}
			}
			if(cid){
				shipping.getShippingRecords(cid);
			}
		}
		if(myAutoComp.itemSelectEvent != null){
			myAutoComp.itemSelectEvent.subscribe(respHandler);
		}
	}


	this.sameAsBilling = function(isChecked){

		for(j = 0; j < document.forms['order'].elements.length; j++){

			var id = document.forms['order'].elements[j].id;
			var type = document.forms['order'].elements[j].type;

			if(isChecked && id.substring(0,9) == 'customers'){

				var fldId = id;
				id = id.replace('customers[','');
				id = id.replace(']','');
				id = id.replace('billaddress','customer_shipping[shipaddress') + ']';

				if(dom.inDocument(id)){
					if(type == "select-one"){
						var idx = document.forms['order'].elements[j].selectedIndex;
						var val = document.forms['order'].elements[j].options[idx].value;
						customer.pickValue(dom.get(id),val);
					}
					else{
						dom.get(id).value = document.forms['order'].elements[j].value;
					}
				}
			}
			else{
				if(!isChecked && id.substring(0,17) == 'customer_shipping'){
					if(type == "select-one"){
						dom.get(id).selectedValue = 0;
					}
					else{
						dom.get(id).value = "";
					}
				}
			}
		}
	}


	this.pickValue = function(oFld,val){

		for(i=0; i<oFld.length; i++) {
			if(oFld[i].value == val) {
				if(oFld.type == "radio"){
					oFld[i].checked = true;
				}
				if(oFld.type == "select-one"){
					oFld.options[i].selected = true;
				}
				break;
			}
		}
	}

}

/*
This class does the auto-complete lookup selections for customer shipping fields
in the phone.orders.php form.
*/

var shipping = new function(){

	// customer shipping fields
	this.fields = new Array("cid",
							"csid",
							"shipaddress_lastname",
							"shipaddress_firstname",
							"shipaddress_companyname",
							"shipaddress_addr1",
							"shipaddress_addr2",
							"shipaddress_city",
							"shipaddress_state",
							"shipaddress_county",
							"shipaddress_postalcode",
							"shipaddress_country",
							"shipaddress_areacode",
							"shipaddress_phone",
							"shipaddress_email");

	this.data = new Array();

	// queries for customer shipping data
	var handleSuccess = function(o){
		if(o.status == 200 && o.responseText !== undefined){
			shipping.data = eval(o.responseText);
			if(typeof shipping.data == 'object' && shipping.data.length > 0){
				shipping.showResults(0);
			}
			else{
				alert("no shipping data");
			}
		}
	}
	var handleFailure = function(o){
		alert('unable to connect to request.php to get shipping records');
	}
	var callback = {
		success: handleSuccess,
		failure: handleFailure
	}
	this.getShippingRecords = function(cid){
		var flds = shipping.fields.join(',');
		var strURL = "../request.php?query=" + cid + "&table=customer_shipping&fields=" + flds;
		var request = YAHOO.util.Connect.asyncRequest('GET', strURL, callback);
	}


	// populates the shipto fields and the address book pull-down if there is more than one address.
	this.showResults = function(index){

		var data = [];
		for(i=0;i<shipping.data.length;i++){
			var flds = shipping.data[i];
			if(parseInt(i) == index){
				for(var j in flds){
					var fldName = 'customer_shipping[' + j + ']';
					var isFld = dom.get(fldName);
					if(isFld != null){
						isFld.value = flds[j];
					}
				}
			}
			data[i] = {	"csid":flds.csid, "address":flds.shipaddress_addr1, "city":flds.shipaddress_city };
		}
		if(data.length > 1){
			shipping.populateSelect(data,index);
		}
	}

	// populates and displays the address book if there is more than one address.
	this.populateSelect = function(data,index){
		// reverse delete existing options
		var selectFld = dom.get('customer_shipping[csid]');
		for( i = (selectFld.length -1); i>=0; i--){
			selectFld.options[i] = null;
		}
		// add options and display
		for(j=0;j<data.length;j++){
			var csid = data[j].csid;
			var addr = data[j].address + ', ' + data[j].city;
			if(csid != null){
				selectFld.options[j] = new Option(addr,csid,false,false);
			}
		}
		selectFld.selectedIndex = index;
		dom.setStyle(dom.get('shipSelect'),'display','');
	}

}

/*
This class does the lookup for the customer sales tax
in the phone.orders.php form.
*/

var salestax = new function(){

	this.data = new Array();
	this.amount = parseFloat(0).toFixed(2);

	// queries for customer shipping data
	var handleSuccess = function(o){
		if(o.status == 200 && o.responseText !== undefined){
			salestax.data = eval(o.responseText);
			if(typeof salestax.data == 'object' && salestax.data.length > 0){
				salestax.amount = parseFloat(salestax.data[0]['salestax']).toFixed(2);
			}
			else{
				salestax.amount = parseFloat(0).toFixed(2);
			}
			dom.get('orders[salestax]').value = salestax.amount;
		}
	}
	var handleFailure = function(o){
		alert('unable to connect to request.php to get salestax records');
	}
	var callback = {
		success: handleSuccess,
		failure: handleFailure
	}
	this.getRecords = function(){

		var isTaxableFld = dom.get('customers[is_taxable]');
		var idx = isTaxableFld.selectedIndex;
		var isTaxable = isTaxableFld.options[idx].value;

		if(isTaxable == 'true' && cart.subtotal > 0){
			var strURL = "../request.php?gettax=true";
			strURL = strURL + "&country=" + dom.get('customer_shipping[shipaddress_country]').value;
			//strURL = strURL + "&county=" + dom.get('customer_shipping[shipaddress_county]').value;
			strURL = strURL + "&state=" + dom.get('customer_shipping[shipaddress_state]').value;
			strURL = strURL + "&zip=" + dom.get('customer_shipping[shipaddress_postalcode]').value;
			strURL = strURL + "&subtotal=" + cart.subtotal;
			var request = YAHOO.util.Connect.asyncRequest('GET', strURL, callback);
		}
		else{
			dom.get('orders[salestax]').value = parseFloat(0).toFixed(2);
		}
	}
}














