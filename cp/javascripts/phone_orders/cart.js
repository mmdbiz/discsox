
/*
This class does the auto-complete lookup selections for cart items
in the phone.orders.php form.
*/

var cart = new function(){

	this.isIE = false;
	this.ua = navigator.userAgent;
	if (this.ua.indexOf('MSIE') >= 0) {
		this.isIE = true;
	}

	this.subtotal = 0;
	// count of products returned for auto-lookup
	this.resultCount = 0;

	// initialize auto-complete fields
	this.resultFld = 'cartLookupContainer';

	// get an HTML template of the first empty row for
	// the cart values so we can append them when
	// the user adds a row to the phone order cart table
	this.cartRowEl = dom.get('cartrow');
	this.cartRowHTML = this.cartRowEl.innerHTML;

	this.rowCount = 1;

	// Setup the XHR DataSource for all cart queries
	this.dataSource = new YAHOO.widget.DS_XHR("../request.php", ["records", "sku", "name", "price"]);
	// configure the response type to be JSON (default)
	this.dataSource.responseType = YAHOO.widget.DS_XHR.TYPE_JSON;
	this.dataSource.maxCacheEntries = 50;
	this.dataSource.scriptQueryAppend = "table=products&fields=sku,name,price&max=50";

	// counts the number of rows in the cart
	this.getRowCount = function(){
		var count = 0;
		for(i=0;i<cart.cartRowEl.childNodes.length;i++){
			var tag = document.getElementById('cartrow').childNodes[i].tagName;
			if(tag == 'TR'){
				count++;
			}
		}
		return parseInt(count);
	}

	this.countRows = function(){
		var rows = 1;
		// workaround: firefox and IE don't agree on childNodes.length
		if(cart.isIE){
			rows = cart.cartRowEl.childNodes.length;
		}
		else{
			rows = cart.getRowCount();
		}
		return rows;
	}


	// this adds the HTML of a row to the phone order cart table
	this.addCartRow = function(){
		var rows = cart.countRows();
		var newRow = cart.cartRowHTML;
		newRow = newRow.replace(/\[0\]/g,'['+rows+']');
		//alert(newRow);
		YAHOO.ext.DomHelper.insertHtml('beforeEnd', cart.cartRowEl, newRow);
		cart.rowCount = rows + 1;
	}

	// removes a row from the cart
	this.removeRow = function(rowId){

		// pulls out just the number of the remove[XX]
		var removeId = rowId.replace('remove[','');
		removeId = removeId.substring(0,removeId.length-1);
		rowId = 'row' + '[' + removeId + ']';

		var cartRow = dom.get('cartrow');
		var rowRemoved = false;

		for (var j = cartRow.childNodes.length-1; j>=0; j--) {
			//alert(cartRow.childNodes.item(j).id);
			if(cartRow.childNodes.item(j).id == rowId && rowId != 'row[0]'){
				cartRow.removeChild(cartRow.childNodes.item(j));
				rowRemoved = true;
				break;
			}
		}

		// renumber all the fields in each row so they come out correctly
		if(rowRemoved){

			// reload the cartrow since we removed rows
			cartRow = dom.get('cartrow');
			var rows = cartRow.getElementsByTagName("TR");
			var subtotal = 0;

			for(i=0;i<rows.length;i++){

				// pulls out just the number of the rowid[]
				var rid = rows[i].id.replace('row[','');
				rid = rid.substring(0,rid.length-1);

				// setup pattern for old id and
				// setup newid based on the row number
				var oldid = '\\[' + rid + '\\]';
				var re = new RegExp(oldid, 'g');
				var newId = '[' + i + ']';

				// set the id for this row
				rows[i].id = 'row' + newId;

				// get all input tags
				var inputs = rows[i].getElementsByTagName("INPUT");

				// loop and reset the name and id attributes
				for(k=0;k<inputs.length;k++){

					// rebuild the id and name attributes
					tagid = inputs[k].id.replace(re,newId);
					inputs[k].id = tagid;
					inputs[k].name = tagid;

					// reset the subtotal
					if(tagid.substring(0,5) == 'total' && inputs[k].value != ""){
						subtotal += parseFloat(inputs[k].value);
					}
				}
			}
			document.getElementById('orders[subtotal]').value = commify(parseFloat(subtotal).toFixed(2));
			cart.subtotal = parseFloat(subtotal).toFixed(2);

			if(dom.get('customer_shipping[shipaddress_country]').value != ""){
				salestax.getRecords();
			}
		}
	}


	// this resets the auto-complete cart fields names
	// when the sku field is clicked. The field names are
	// php array style fields like sku[0],description[0], etc.
	// This way we can show the auto-complete for each row.
	this.initCartFields = function(fldName){

		// pulls out just the number from the clicked sku field (sku[0])
		var pattern = /\[(\d+)\]/g;
		var intResult = parseInt(strip(fldName.match(pattern)));

		// reset the fields for the auto-complete functions
		skuFld = 'sku[' + intResult + ']';
		nameFld = 'description[' + intResult + ']';
		priceFld = 'price[' + intResult + ']';
		qtyPriceFld = 'qtyprice[' + intResult + ']';
		qtyFld = 'quantity[' + intResult + ']';
		totalFld = 'total[' + intResult + ']';

		// set the AC to the new field
		var myAutoComp = new YAHOO.widget.AutoComplete(skuFld,cart.resultFld,cart.dataSource);
		// AC options
		myAutoComp.queryDelay = 1;
		myAutoComp.minQueryLength = 2;
		myAutoComp.maxResultsDisplayed = 50;
		myAutoComp.autoHighlight = false;
		myAutoComp.useShadow = true;

		myAutoComp.formatResult = function(aResultItem, sQuery) {
			var sku = aResultItem[0];
			var name = aResultItem[1];
			var price = cart.calculateQuantityPrice(aResultItem[2],1);
			return sku + ' - ' + name + ' - ' + parseFloat(price).toFixed(2);
		}

		myAutoComp.doBeforeExpandContainer = function(Textbox, Container, sQuery, aResults){
			var cont = document.getElementById('cartLookupContainer');
			var len = 415 - (aResults.length * 8);
			cont.style.top = Math.round(len) + 'px';
			return true;
		}

		var respHandler = function(type, args, me) {
							var oAutoComp = args[0];  // the autocomplete instance
							var elListItem = args[1]; // the result list item element
							var sku = args[2][0];
							var name = args[2][1];
							var price = args[2][2];
							var qtyprice = args[2][2];
							
							document.getElementById(skuFld).value = sku;
							document.getElementById(nameFld).value = name;
							document.getElementById(priceFld).value = cart.calculateQuantityPrice(price,1);
							document.getElementById(qtyPriceFld).value = qtyprice;
							document.getElementById(qtyFld).value = 1;
							document.getElementById(totalFld).value = cart.calculateQuantityPrice(price,1);
							// auto-select quantity to speed up input for the user
							document.getElementById(qtyFld).select();
						 }
						 if(myAutoComp.itemSelectEvent != null){
							myAutoComp.itemSelectEvent.subscribe(respHandler);
						 }
	}

	this.setTotals = function(){

		var price = dom.get(priceFld).value;
		var qtyprice = dom.get(qtyPriceFld).value;
		var qty = dom.get(qtyFld).value;

		if(qtyprice.indexOf(':') > 0){
			price = cart.calculateQuantityPrice(qtyprice,qty);
			dom.get(priceFld).value = price;
		}
		
		dom.get(totalFld).value = parseFloat(price * qty).toFixed(2);

		var subtotal = 0;

		for(i=0;i<cart.rowCount;i++){
			var tFld = 'total[' + i + ']';
			if(document.getElementById(tFld).value != ''){
				subtotal += parseFloat(document.getElementById(tFld).value);
			}
		}
		dom.get('orders[subtotal]').value = commify(parseFloat(subtotal).toFixed(2));
		cart.subtotal = parseFloat(subtotal).toFixed(2);

		if(dom.get('customer_shipping[shipaddress_country]').value != ""){
			salestax.getRecords();
		}

		setTimeout('cart.displayTotals()',1000);

	}


	this.displayTotals = function(){

		var subtotal = parseFloat(cart.subtotal).toFixed(2);

		var tax = parseFloat(0);
		var discount = parseFloat(0);
		var shipping = parseFloat(0);

		if(dom.get('orders[discount]').value != ""){
			discount = parseFloat(dom.get('orders[discount]').value);
			dom.get('orders[discount]').value = commify(parseFloat(discount).toFixed(2));
		}
		else{
			discount = parseFloat(0).toFixed(2);
			dom.get('orders[discount]').value = commify(parseFloat(0).toFixed(2));
		}

		if(dom.get('orders[shipping]').value != ""){
			shipping = parseFloat(dom.get('orders[shipping]').value);
			dom.get('orders[shipping]').value = commify(parseFloat(shipping).toFixed(2));
		}
		else{
			shipping = parseFloat(0).toFixed(2);
			dom.get('orders[shipping]').value = commify(parseFloat(0).toFixed(2));
		}

		if(dom.get('orders[salestax]').value != ""){
			tax = parseFloat(dom.get('orders[salestax]').value);
			dom.get('orders[salestax]').value = commify(parseFloat(tax).toFixed(2));
		}
		else{
			tax = parseFloat(0).toFixed(2);
			dom.get('orders[salestax]').value = commify(parseFloat(0).toFixed(2));
		}

		var grandTotal = ((parseFloat(subtotal) - parseFloat(discount)) + parseFloat(shipping)) + parseFloat(tax);

		dom.get('orders[grandtotal]').value = commify(parseFloat(grandTotal).toFixed(2));
	}


	this.calculateQuantityPrice = function(price,quantity){
		if (price.indexOf(':')){
			var qtyPrices = price.split(',');
			for(var i=0;i<qtyPrices.length;i++){
				var flds = qtyPrices[i].split(':');
				var qty = flds[0];
				var prc = flds[1];
				if (qty.indexOf('-') >= 0) {
					var ranges = qty.split('-');
					var low = ranges[0];
					var high = 999999;
					if(ranges.count == 2){
						high = ranges[1];
					}
					if(quantity >= low && quantity <= high){
						price = parseFloat(prc).toFixed(2);
					}
				}
				else{
					if(quantity >= qty){
						price = parseFloat(prc).toFixed(2);
					}
				}
			}
		}
		else{
			price = parseFloat(price).toFixed(2);
		}
		return price;
	}



}




















