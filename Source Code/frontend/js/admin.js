// Admin portal logic for system supervision, manager management, and revenue analytics
let currentAdmin = null;
let currentAdminFilter = 'All';
let adminVehiclesList = [];

document.addEventListener('DOMContentLoaded', () => {
  currentAdmin = api.checkAuth(['Admin']);
  if (!currentAdmin) return;

  loadRevenueReport();
  loadManagers();
  loadAdminPendingOrders();
  loadAdminVehicles();
  loadAdminRentedCustomers();
});

// Switch between Admin tabs: Revenue, Managers, Pending Orders, Vehicles Inventory, Rented Customers
function switchAdminTab(tab) {
  const tabs = ['revenue', 'managers', 'orders', 'vehicles', 'rented'];
  tabs.forEach(t => {
    const panel = document.getElementById(`tabPanel${capitalizeFirst(t)}`);
    const btn = document.getElementById(`tabBtn${capitalizeFirst(t)}`);
    if (panel) panel.style.display = (t === tab) ? 'block' : 'none';
    if (btn) btn.classList.toggle('active', t === tab);
  });

  if (tab === 'revenue') loadRevenueReport();
  if (tab === 'managers') loadManagers();
  if (tab === 'orders') loadAdminPendingOrders();
  if (tab === 'vehicles') loadAdminVehicles();
  if (tab === 'rented') loadAdminRentedCustomers();
}

// Utility to capitalize first character
function capitalizeFirst(textString) {
  return textString.charAt(0).toUpperCase() + textString.slice(1);
}

// Load total revenue report and breakdown metrics
async function loadRevenueReport() {
  const revRes = await api.get('/api/admin/revenue');
  if (revRes.success && revRes.revenue) {
    const r = revRes.revenue;
    document.getElementById('adminTotalRevenue').textContent = api.formatMoney(r.totalRevenue);
    document.getElementById('adminApprovedOrders').textContent = r.approvedOrders;
    document.getElementById('adminPendingOrders').textContent = r.pendingOrders;
    document.getElementById('adminRejectedOrders').textContent = r.rejectedOrders;
  }

  const rentedRes = await api.get('/api/rentals/rented-customers');
  const tbody = document.getElementById('revenueTableBody');
  if (!tbody) return;

  if (!rentedRes.success || !rentedRes.data || rentedRes.data.length === 0) {
    tbody.innerHTML = `<tr><td colspan="8" style="text-align:center; padding:2rem; color:var(--gray);">
      No rental orders approved for revenue calculation yet.
    </td></tr>`;
    return;
  }

  tbody.innerHTML = '';
  rentedRes.data.forEach(item => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><strong>#${item.orderID}</strong></td>
      <td><strong>${item.customerName}</strong></td>
      <td>${item.vehicleInfo}</td>
      <td><span class="licensePlatePill">${item.licensePlate}</span></td>
      <td>${item.startDate}</td>
      <td>${item.endDate}</td>
      <td style="color:#059669; font-weight:700;">${api.formatMoney(item.totalAmount)}</td>
      <td><span class="statusBadge statusApproved">Successful</span></td>
    `;
    tbody.appendChild(tr);
  });
}

// Load manager accounts table
async function loadManagers() {
  const res = await api.get('/api/admin/managers');
  const tbody = document.getElementById('managerTableBody');
  if (!tbody) return;

  if (!res.success || !res.data || res.data.length === 0) {
    tbody.innerHTML = `<tr><td colspan="4" style="text-align:center; padding:2rem;">No manager accounts found.</td></tr>`;
    document.getElementById('adminManagerCount').textContent = 0;
    return;
  }

  document.getElementById('adminManagerCount').textContent = res.data.length;
  tbody.innerHTML = '';

  res.data.forEach(m => {
    const mgrId = m.id || m.managerId;
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><strong>#${mgrId}</strong></td>
      <td><strong>${m.username}</strong></td>
      <td><span class="userRoleBadge">Manager</span></td>
      <td style="text-align:center;">
        <button class="btn btnDanger btnSm" onclick="handleDeleteManager(${mgrId}, '${m.username}')">
          Delete Manager
        </button>
      </td>
    `;
    tbody.appendChild(tr);
  });
}

// Handle create new manager form submission
async function handleCreateManager(event) {
  event.preventDefault();
  const username = document.getElementById('newMgrUser').value.trim();
  const password = document.getElementById('newMgrPass').value.trim();

  const res = await api.post('/api/admin/managers/add', { username, password });
  if (res.success) {
    alert(res.message);
    document.getElementById('createManagerForm').reset();
    loadManagers();
  } else {
    alert(res.message || 'Error creating Manager account!');
  }
}

// Handle delete manager account
async function handleDeleteManager(managerId, username) {
  if (!confirm(`Confirm DELETE Manager account "${username}" (ID #${managerId})?`)) return;

  const res = await api.post('/api/admin/managers/delete', { managerId });
  if (res.success) {
    alert(res.message);
    loadManagers();
  } else {
    alert(res.message || 'Error deleting Manager account!');
  }
}

// Load pending rental orders for Admin approval
async function loadAdminPendingOrders() {
  const tbody = document.getElementById('adminPendingOrdersBody');
  if (!tbody) return;

  const res = await api.get('/api/rentals/pending');

  if (!res.success || !res.data || res.data.length === 0) {
    tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding:2rem; color:var(--gray);">
      There are currently no vehicle rental requests awaiting approval.
    </td></tr>`;
    return;
  }

  tbody.innerHTML = '';
  res.data.forEach(o => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><strong>#${o.orderID}</strong></td>
      <td><strong>${o.customerUsername || o.customerName}</strong></td>
      <td>${o.vehicleBrand} ${o.vehicleModel} (<span class="licensePlatePill">${o.licensePlate}</span>)</td>
      <td>National ID: <strong>${o.cccd}</strong><br>License: <strong>${o.driverLicense}</strong></td>
      <td>${o.startDate} - ${o.endDate}</td>
      <td style="color:var(--primary); font-weight:700;">${api.formatMoney(o.totalAmount)}</td>
      <td style="text-align:center;">
        <div style="display:flex; gap:8px; justify-content:center;">
          <button class="btn btnSuccess btnSm" onclick="adminApproveOrder(${o.orderID})">Approve</button>
          <button class="btn btnDanger btnSm" onclick="adminRejectOrder(${o.orderID})">Reject</button>
        </div>
      </td>
    `;
    tbody.appendChild(tr);
  });
}

// Admin approve rental order
async function adminApproveOrder(orderId) {
  if (!confirm(`Admin approve rental order #${orderId}?`)) return;
  const res = await api.post('/api/rentals/approve', { orderId });
  if (res.success) {
    alert(res.message);
    loadRevenueReport();
    loadAdminPendingOrders();
    loadAdminVehicles();
  }
}

// Admin reject rental order
async function adminRejectOrder(orderId) {
  if (!confirm(`Confirm REJECT rental order #${orderId}?`)) return;
  const res = await api.post('/api/rentals/reject', { orderId });
  if (res.success) {
    alert(res.message);
    loadRevenueReport();
    loadAdminPendingOrders();
  }
}

// Load vehicles list for Admin inventory view
async function loadAdminVehicles() {
  const res = await api.get('/api/vehicles/all');
  const tbody = document.getElementById('adminVehicleTableBody');
  if (!tbody) return;

  if (!res.success || !res.data) return;
  adminVehiclesList = res.data;
  tbody.innerHTML = '';

  const filtered = res.data.filter(v => currentAdminFilter === 'All' || v.type === currentAdminFilter);

  filtered.forEach(v => {
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
          ${isAvail ? 'Available' : 'Currently Rented'}
        </span>
      </td>
    `;
    tbody.appendChild(tr);
  });
}

// Filter Admin vehicle view
function filterAdminVehicles(type) {
  currentAdminFilter = type;
  document.getElementById('adminFilterAll').classList.toggle('active', type === 'All');
  document.getElementById('adminFilterCar').classList.toggle('active', type === 'Car');
  document.getElementById('adminFilterMoto').classList.toggle('active', type === 'Motobike');
  loadAdminVehicles();
}

// Load list of rented customers for Admin view
async function loadAdminRentedCustomers() {
  const tbody = document.getElementById('adminRentedBody');
  if (!tbody) return;

  const res = await api.get('/api/rentals/rented-customers');

  if (!res.success || !res.data || res.data.length === 0) {
    tbody.innerHTML = `<tr><td colspan="8" style="text-align:center; padding:2rem; color:var(--gray);">
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
      <td><strong>${c.customerName}</strong></td>
      <td>National ID: <strong>${c.cccd}</strong><br>License: <strong>${c.driverLicense}</strong></td>
      <td><strong>${c.vehicleInfo}</strong></td>
      <td><span class="licensePlatePill">${c.licensePlate}</span></td>
      <td>${c.startDate} - ${c.endDate}</td>
      <td style="font-weight:700; color:#059669;">${api.formatMoney(c.totalAmount)}</td>
      <td>
        <span class="statusBadge ${isCompleted ? 'statusCompleted' : 'statusApproved'}">
          ${isCompleted ? 'Completed' : 'In Progress'}
        </span>
      </td>
    `;
    tbody.appendChild(tr);
  });
}
