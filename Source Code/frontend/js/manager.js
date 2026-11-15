// Manager portal logic and inventory management
let currentUser = null;
let currentManagerFilter = 'All';
let allVehiclesList = [];

document.addEventListener('DOMContentLoaded', () => {
  currentUser = api.checkAuth(['Manager', 'Admin']);
  if (!currentUser) return;

  loadStats();
  loadPendingOrders();
  loadVehicles();
  loadRentedCustomers();
});

// Switch between Manager tabs: Orders, Vehicles, Rented Customers
function switchManagerTab(tab) {
  const panelOrders = document.getElementById('tabPanelOrders');
  const panelVehicles = document.getElementById('tabPanelVehicles');
  const panelRented = document.getElementById('tabPanelRented');

  const btnOrders = document.getElementById('tabBtnOrders');
  const btnVehicles = document.getElementById('tabBtnVehicles');
  const btnRented = document.getElementById('tabBtnRented');

  panelOrders.style.display = (tab === 'orders') ? 'block' : 'none';
  panelVehicles.style.display = (tab === 'vehicles') ? 'block' : 'none';
  panelRented.style.display = (tab === 'rented') ? 'block' : 'none';

  btnOrders.classList.toggle('active', tab === 'orders');
  btnVehicles.classList.toggle('active', tab === 'vehicles');
  btnRented.classList.toggle('active', tab === 'rented');

  if (tab === 'orders') loadPendingOrders();
  if (tab === 'vehicles') loadVehicles();
  if (tab === 'rented') loadRentedCustomers();
}

// Load inventory counts and pending order metrics
async function loadStats() {
  const cRes = await api.get('/api/vehicles/count');
  if (cRes.success && cRes.counts) {
    document.getElementById('statTotalVehicles').textContent = cRes.counts.total;
    document.getElementById('statAvailableCars').textContent = cRes.counts.availableCars;
    document.getElementById('statAvailableMotos').textContent = cRes.counts.availableMotobikes;
  }

  const pRes = await api.get('/api/rentals/pending');
  if (pRes.success && pRes.data) {
    document.getElementById('statPendingCount').textContent = pRes.data.length;
  }
}

// Load pending rental requests awaiting approval
async function loadPendingOrders() {
  const tbody = document.getElementById('pendingOrdersBody');
  if (!tbody) return;

  const res = await api.get('/api/rentals/pending');

  if (!res.success || !res.data || res.data.length === 0) {
    tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 2rem; color: var(--gray);">
      There are currently no vehicle rental requests awaiting approval.
    </td></tr>`;
    document.getElementById('statPendingCount').textContent = 0;
    return;
  }

  document.getElementById('statPendingCount').textContent = res.data.length;
  tbody.innerHTML = '';

  res.data.forEach(o => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><strong>#${o.orderID}</strong></td>
      <td>
        <strong>${o.customerUsername || o.customerName}</strong><br>
        <small style="color:var(--gray);">ID: ${o.customerID}</small>
      </td>
      <td>
        <strong>${o.vehicleBrand} ${o.vehicleModel}</strong><br>
        <span class="licensePlatePill">${o.licensePlate}</span>
        <span style="font-size:12px; color:var(--gray); margin-left:4px;">${o.vehicleType === 'Car' ? 'Car' : 'Motorbike'}</span>
      </td>
      <td>
        National ID: <strong>${o.cccd}</strong><br>
        License: <strong>${o.driverLicense}</strong>
      </td>
      <td>${o.startDate} - ${o.endDate}</td>
      <td style="color:var(--primary); font-weight:700;">${api.formatMoney(o.totalAmount)}</td>
      <td style="text-align:center;">
        <div style="display:flex; gap:8px; justify-content:center;">
          <button class="btn btnSuccess btnSm" onclick="approveOrder(${o.orderID}, '${o.vehicleBrand} ${o.vehicleModel}')">
            Approve
          </button>
          <button class="btn btnDanger btnSm" onclick="rejectOrder(${o.orderID}, '${o.customerUsername || o.customerName}')">
            Reject
          </button>
        </div>
      </td>
    `;
    tbody.appendChild(tr);
  });
}

// Approve pending rental request
async function approveOrder(orderId, vehName) {
  if (!confirm(`Confirm APPROVE for vehicle request #${orderId} (${vehName})?`)) return;

  const res = await api.post('/api/rentals/approve', { orderId: orderId });
  if (res.success) {
    alert(res.message);
    loadPendingOrders();
    loadStats();
  } else {
    alert(res.message || 'Error approving order!');
  }
}

// Reject pending rental request
async function rejectOrder(orderId, customerName) {
  if (!confirm(`Confirm REJECT for vehicle request #${orderId} of customer ${customerName}?`)) return;

  const res = await api.post('/api/rentals/reject', { orderId: orderId });
  if (res.success) {
    alert(res.message);
    loadPendingOrders();
    loadStats();
  } else {
    alert(res.message || 'Error rejecting order!');
  }
}

// Load vehicles list for inventory table
async function loadVehicles() {
  const res = await api.get(`/api/vehicles?type=${currentManagerFilter === 'All' ? '' : currentManagerFilter}`);
  const tbody = document.getElementById('vehicleTableBody');
  if (!tbody) return;

  if (!res.success || !res.data || res.data.length === 0) {
    tbody.innerHTML = `<tr><td colspan="8" style="text-align:center; padding: 2rem; color: var(--gray);">
      No vehicles found in this category.
    </td></tr>`;
    return;
  }

  allVehiclesList = res.data;
  tbody.innerHTML = '';

  res.data.forEach(v => {
    const isCar = (v.type === 'Car');
    const isAvail = (v.isavailable == 1);

    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><strong>#${v.vehicleID}</strong></td>
      <td><strong>${v.brand} ${v.model}</strong></td>
      <td><span class="licensePlatePill">${v.licenseplate}</span></td>
      <td>
        <span class="vehicleTypeTag ${isCar ? 'tagCar' : 'tagMoto'}">
          ${isCar ? 'CAR' : 'MOTORBIKE'}
        </span>
      </td>
      <td>${isCar ? `${v.numseat} seats` : `${v.enginecapacity} cc`}</td>
      <td style="font-weight:700; color:var(--primary);">${api.formatMoney(v.priceperday)}</td>
      <td>
        <span class="statusBadge ${isAvail ? 'statusApproved' : 'statusRejected'}">
          ${isAvail ? 'Available' : 'Rented'}
        </span>
      </td>
      <td style="text-align:center;">
        <div style="display:flex; gap:6px; justify-content:center;">
          <button class="btn btnOutline btnSm" onclick="openEditVehicleModal(${v.vehicleID})">Edit</button>
          <button class="btn btnDanger btnSm" onclick="deleteVehicle(${v.vehicleID}, '${v.brand} ${v.model}')">Delete</button>
        </div>
      </td>
    `;
    tbody.appendChild(tr);
  });
}

// Filter vehicles inventory table
function filterMgrVehicles(type) {
  currentManagerFilter = type;
  document.getElementById('mgrFilterAll').classList.toggle('active', type === 'All');
  document.getElementById('mgrFilterCar').classList.toggle('active', type === 'Car');
  document.getElementById('mgrFilterMoto').classList.toggle('active', type === 'Motobike');
  loadVehicles();
}

// Toggle seats or engine capacity input based on vehicle type
function toggleVehicleFields() {
  const type = document.getElementById('vehFormType').value;
  document.getElementById('groupCarSeats').style.display = (type === 'Car') ? 'block' : 'none';
  document.getElementById('groupMotoCc').style.display = (type === 'Motobike') ? 'block' : 'none';
}

// Open modal for adding a new vehicle
function openAddVehicleModal() {
  document.getElementById('vehicleModalTitle').textContent = 'Add New Vehicle to Inventory';
  document.getElementById('vehFormId').value = '';
  document.getElementById('vehicleForm').reset();
  document.getElementById('vehFormType').value = 'Car';
  toggleVehicleFields();
  document.getElementById('vehicleModal').classList.add('active');
}

// Open modal for editing an existing vehicle
function openEditVehicleModal(id) {
  const v = allVehiclesList.find(item => item.vehicleID === id);
  if (!v) return;

  document.getElementById('vehicleModalTitle').textContent = `Edit Vehicle #${id}: ${v.brand} ${v.model}`;
  document.getElementById('vehFormId').value = v.vehicleID;
  document.getElementById('vehFormType').value = v.type;
  document.getElementById('vehFormBrand').value = v.brand;
  document.getElementById('vehFormModel').value = v.model;
  document.getElementById('vehFormPlate').value = v.licenseplate;
  document.getElementById('vehFormPrice').value = v.priceperday;

  if (v.type === 'Car') {
    document.getElementById('vehFormSeats').value = v.numseat;
  } else {
    document.getElementById('vehFormCc').value = v.enginecapacity;
  }

  toggleVehicleFields();
  document.getElementById('vehicleModal').classList.add('active');
}

// Close vehicle form modal
function closeVehicleModal() {
  document.getElementById('vehicleModal').classList.remove('active');
}

// Save vehicle (insert or update)
async function handleSaveVehicle(event) {
  event.preventDefault();
  const idStr = document.getElementById('vehFormId').value;
  const isEdit = (idStr !== '');

  const payload = {
    type: document.getElementById('vehFormType').value,
    brand: document.getElementById('vehFormBrand').value.trim(),
    model: document.getElementById('vehFormModel').value.trim(),
    licensePlate: document.getElementById('vehFormPlate').value.trim(),
    pricePerDay: parseFloat(document.getElementById('vehFormPrice').value)
  };

  if (payload.type === 'Car') {
    payload.numSeats = parseInt(document.getElementById('vehFormSeats').value);
  } else {
    payload.engineCapacity = parseInt(document.getElementById('vehFormCc').value);
  }

  const endpoint = isEdit ? '/api/vehicles/update' : '/api/vehicles/add';
  if (isEdit) {
    payload.id = parseInt(idStr);
  }

  const res = await api.post(endpoint, payload);
  if (res.success) {
    alert(res.message);
    closeVehicleModal();
    loadVehicles();
    loadStats();
  } else {
    alert(res.message || 'Error saving vehicle!');
  }
}

// Delete vehicle from inventory
async function deleteVehicle(id, name) {
  if (!confirm(`Are you sure you want to DELETE vehicle #${id} (${name}) from the system?`)) return;

  const res = await api.post('/api/vehicles/delete', { id: id });
  if (res.success) {
    alert(res.message);
    loadVehicles();
    loadStats();
  } else {
    alert(res.message || 'Error deleting vehicle!');
  }
}

// Load list of customers with active or completed rentals
async function loadRentedCustomers() {
  const tbody = document.getElementById('rentedCustomersBody');
  if (!tbody) return;

  const res = await api.get('/api/rentals/rented-customers');

  if (!res.success || !res.data || res.data.length === 0) {
    tbody.innerHTML = `<tr><td colspan="8" style="text-align:center; padding: 2rem; color: var(--gray);">
      No customers currently renting vehicles.
    </td></tr>`;
    return;
  }

  tbody.innerHTML = '';
  res.data.forEach(c => {
    const isCompleted = (c.status === 'Completed');
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><strong>#${c.orderID}</strong></td>
      <td>
        <strong>${c.customerName}</strong><br>
        <small style="color:var(--gray);">ID: ${c.customerID || 'N/A'}</small>
      </td>
      <td>
        National ID: <strong>${c.cccd}</strong><br>
        License: <strong>${c.driverLicense}</strong>
      </td>
      <td><strong>${c.vehicleInfo}</strong></td>
      <td><span class="licensePlatePill">${c.licensePlate}</span></td>
      <td>${c.startDate} - ${c.endDate}</td>
      <td style="font-weight:700; color:var(--primary);">${api.formatMoney(c.totalAmount)}</td>
      <td>
        <span class="statusBadge ${isCompleted ? 'statusCompleted' : 'statusApproved'}">
          ${isCompleted ? 'Returned' : 'Rented'}
        </span>
      </td>
    `;
    tbody.appendChild(tr);
  });
}

// Dismiss manager modal on overlay click
function handleManagerOverlay(event, modalId) {
  if (event.target.id === modalId) {
    document.getElementById(modalId).classList.remove('active');
  }
}

// Close modal on Escape key
document.addEventListener('keydown', (event) => {
  if (event.key === 'Escape') {
    closeVehicleModal();
  }
});
