const API_BASE = "http://127.0.0.1:8080/api";
const TOKEN_KEY = "vehicle-rental-token";
const ROLE_PAGES = {
  Customer: "customer.html",
  Manager: "manager.html",
  Admin: "admin.html"
};
const money = new Intl.NumberFormat("vi-VN", {
  style: "currency",
  currency: "VND",
  maximumFractionDigits: 0
});
const dateFormat = new Intl.DateTimeFormat("vi-VN", { dateStyle: "medium" });

function element(tag, className, text) {
  const node = document.createElement(tag);
  if (className) node.className = className;
  if (text !== undefined) node.textContent = text;
  return node;
}

function showMessage(message, kind = "success", targetId = "page-message") {
  const target = document.getElementById(targetId);
  if (!target) return;
  target.textContent = message;
  target.className = `notice notice-${kind}`;
  target.hidden = false;
  target.scrollIntoView({ behavior: "smooth", block: "nearest" });
}

async function api(path, options = {}) {
  const headers = new Headers(options.headers || {});
  const token = localStorage.getItem(TOKEN_KEY);
  if (token) headers.set("Authorization", `Bearer ${token}`);
  if (options.body && !headers.has("Content-Type")) {
    headers.set("Content-Type", "application/json");
  }
  const response = await fetch(`${API_BASE}${path}`, { ...options, headers });
  const payload = await response.json();
  if (response.status === 401 && document.body.dataset.page !== "auth") {
    localStorage.removeItem(TOKEN_KEY);
    window.location.replace("index.html");
  }
  if (!response.ok) throw new Error(payload.error || "Không thể hoàn tất yêu cầu.");
  return payload;
}

function redirectForRole(role) {
  const page = ROLE_PAGES[role];
  if (!page) throw new Error("Tài khoản không có vai trò hợp lệ.");
  window.location.replace(page);
}

async function initializeAuthPage() {
  const token = localStorage.getItem(TOKEN_KEY);
  if (token) {
    try {
      const { user } = await api("/me");
      redirectForRole(user.role);
      return;
    } catch {
      localStorage.removeItem(TOKEN_KEY);
    }
  }

  const tabs = [...document.querySelectorAll("[data-auth-tab]")];
  const loginForm = document.getElementById("login-form");
  const registerForm = document.getElementById("register-form");
  const title = document.getElementById("auth-title");
  const description = document.getElementById("auth-description");

  function setMode(mode) {
    const registering = mode === "register";
    loginForm.hidden = registering;
    registerForm.hidden = !registering;
    title.textContent = registering ? "Tạo tài khoản" : "Đăng nhập";
    description.textContent = registering
      ? "Đăng ký để bắt đầu lên kế hoạch cho chuyến đi."
      : "Đăng nhập để tiếp tục hành trình.";
    tabs.forEach((tab) => {
      const selected = tab.dataset.authTab === mode;
      tab.classList.toggle("is-active", selected);
      tab.setAttribute("aria-selected", String(selected));
    });
    document.getElementById("auth-message").hidden = true;
  }

  tabs.forEach((tab) => tab.addEventListener("click", () => setMode(tab.dataset.authTab)));
  loginForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    const button = loginForm.querySelector('button[type="submit"]');
    button.disabled = true;
    try {
      const { token: sessionToken, user } = await api("/auth/login", {
        method: "POST",
        body: JSON.stringify(Object.fromEntries(new FormData(loginForm)))
      });
      localStorage.setItem(TOKEN_KEY, sessionToken);
      redirectForRole(user.role);
    } catch (error) {
      showMessage(error.message, "error", "auth-message");
    } finally {
      button.disabled = false;
    }
  });
  registerForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    const button = registerForm.querySelector('button[type="submit"]');
    button.disabled = true;
    try {
      await api("/auth/register", {
        method: "POST",
        body: JSON.stringify(Object.fromEntries(new FormData(registerForm)))
      });
      setMode("login");
      loginForm.elements.username.value = registerForm.elements.username.value;
      showMessage("Đăng ký thành công. Hãy đăng nhập bằng tài khoản vừa tạo.", "success", "auth-message");
      registerForm.reset();
    } catch (error) {
      showMessage(error.message, "error", "auth-message");
    } finally {
      button.disabled = false;
    }
  });
}

async function requirePageRole(expectedRole) {
  if (!localStorage.getItem(TOKEN_KEY)) {
    window.location.replace("index.html");
    return null;
  }
  try {
    const { user } = await api("/me");
    if (user.role !== expectedRole) {
      redirectForRole(user.role);
      return null;
    }
    const username = document.getElementById("current-user");
    const avatar = document.getElementById("user-avatar");
    if (username) username.textContent = user.username;
    if (avatar) avatar.textContent = [...user.username][0]?.toLocaleUpperCase("vi-VN") || "L";
    const welcome = document.getElementById("welcome-name");
    if (welcome) welcome.textContent = user.username;
    return user;
  } catch (error) {
    if (error.message && !error.message.includes("đăng nhập")) {
      showMessage(error.message, "error");
    }
    return null;
  }
}

function attachLogout() {
  document.querySelectorAll("[data-logout]").forEach((button) => {
    button.addEventListener("click", async () => {
      button.disabled = true;
      try {
        await api("/auth/logout", { method: "POST" });
      } catch (error) {
        console.error("Could not revoke the server session during logout:", error);
      } finally {
        localStorage.removeItem(TOKEN_KEY);
        window.location.replace("index.html");
      }
    });
  });
}

function statusLabel(status) {
  return {
    Pending: ["Chờ duyệt", "status-pending"],
    Approved: ["Đã duyệt", "status-approved"],
    Rejected: ["Đã từ chối", "status-rejected"],
    Completed: ["Hoàn tất", "status-completed"]
  }[status] || [status, "status-pending"];
}

function appendStatus(parent, status) {
  const [label, className] = statusLabel(status);
  parent.append(element("span", `status-pill ${className}`, label));
}

async function loadVehicles(type = "all") {
  const container = document.getElementById("available-vehicles");
  const counts = document.getElementById("vehicle-counts");
  if (!container) return;
  container.replaceChildren(element("div", "loading-card", "Đang tải danh sách xe…"));
  try {
    const result = await api(`/vehicles?type=${encodeURIComponent(type)}`);
    container.replaceChildren();
    const cards = result.items.map((vehicle) => {
      const card = element("article", "vehicle-card");
      const tag = element("span", "vehicle-type", vehicle.type === "Car" ? "Ô TÔ" : "XE MÁY");
      const heading = element("h3", "", `${vehicle.brand} ${vehicle.model}`);
      const plate = element("p", "vehicle-plate", `Biển số · ${vehicle.licensePlate}`);
      const facts = element("div", "vehicle-facts");
      facts.append(element("span", "", vehicle.type === "Car"
        ? `${vehicle.seatCount || "—"} chỗ`
        : `${vehicle.engineCapacity || "—"} cc`));
      const price = element("strong", "vehicle-price", money.format(vehicle.pricePerDay));
      price.append(element("span", "", " / ngày"));
      const button = element("button", "button button-secondary button-small", "Chọn xe này");
      button.type = "button";
      button.addEventListener("click", () => selectVehicle(vehicle));
      card.append(tag, heading, plate, facts, price, button);
      return card;
    });
    if (cards.length) container.append(...cards);
    else container.append(element("div", "empty-state", "Chưa có xe phù hợp đang sẵn sàng."));

    if (counts) {
      counts.replaceChildren(
        element("span", "count-chip", `Tổng cộng ${result.counts.total} xe`),
        element("span", "count-chip", `${result.counts.car} ô tô`),
        element("span", "count-chip", `${result.counts.motorbike} xe máy`)
      );
    }
  } catch (error) {
    container.replaceChildren(element("div", "empty-state", error.message));
  }
}

function selectVehicle(vehicle) {
  const form = document.getElementById("rental-form");
  if (!form) return;
  form.elements.vehicleId.value = vehicle.id;
  const selected = document.getElementById("selected-vehicle");
  selected.replaceChildren(
    element("strong", "", `${vehicle.brand} ${vehicle.model} · ${money.format(vehicle.pricePerDay)}/ngày`),
    element("span", "", `Biển số ${vehicle.licensePlate}`)
  );
  form.scrollIntoView({ behavior: "smooth", block: "center" });
}

async function submitRental(event) {
  event.preventDefault();
  const form = event.currentTarget;
  if (!form.elements.vehicleId.value) {
    showMessage("Vui lòng chọn một xe trước khi gửi yêu cầu.", "error");
    document.getElementById("available-vehicles").scrollIntoView({ behavior: "smooth" });
    return;
  }
  const start = new Date(`${form.elements.startDate.value}T00:00:00`);
  const end = new Date(`${form.elements.endDate.value}T00:00:00`);
  if (!(start < end)) {
    showMessage("Ngày trả xe phải sau ngày nhận xe.", "error");
    return;
  }
  const button = form.querySelector('button[type="submit"]');
  button.disabled = true;
  try {
    const result = await api("/rentals", {
      method: "POST",
      body: JSON.stringify({
        vehicleId: Number(form.elements.vehicleId.value),
        cccd: form.elements.cccd.value.trim(),
        driverLicense: form.elements.driverLicense.value.trim(),
        startDate: form.elements.startDate.value,
        endDate: form.elements.endDate.value
      })
    });
    showMessage(
      `Đã gửi đơn #${result.id}. Tiền cọc mô phỏng ${money.format(result.depositAmount)} đã được ghi nhận.`,
      "success"
    );
    form.reset();
    document.getElementById("selected-vehicle").replaceChildren(
      document.createTextNode("Chưa chọn xe "),
      element("span", "", "Chọn một xe ở danh sách phía trên")
    );
    await Promise.all([loadVehicles(), loadHistory(), loadNotifications()]);
  } catch (error) {
    showMessage(error.message, "error");
  } finally {
    button.disabled = false;
  }
}

async function loadHistory() {
  const body = document.getElementById("rental-history");
  if (!body) return;
  try {
    const rows = await api("/rentals/mine");
    body.replaceChildren();
    if (!rows.length) {
      body.append(element("tr", "", ""));
      body.firstElementChild.append(element("td", "empty-cell", "Bạn chưa có lịch sử thuê xe."));
      body.firstElementChild.firstElementChild.colSpan = 6;
      return;
    }
    rows.forEach((rental) => {
      const row = element("tr");
      row.append(
        element("td", "table-strong", `#${rental.id}`),
        element("td", "", `${rental.brand} ${rental.model}`),
        element("td", "", `${rental.startDate} – ${rental.endDate}`),
        element("td", "", money.format(rental.totalAmount)),
        element("td", "", money.format(rental.depositAmount))
      );
      const status = element("td");
      appendStatus(status, rental.status);
      row.append(status);
      body.append(row);
    });
  } catch (error) {
    body.replaceChildren();
    const row = element("tr");
    const cell = element("td", "empty-cell", error.message);
    cell.colSpan = 6;
    row.append(cell);
    body.append(row);
  }
}

async function loadNotifications() {
  const list = document.getElementById("notifications-list");
  if (!list) return;
  try {
    const notifications = await api("/notifications");
    list.replaceChildren();
    if (!notifications.length) {
      list.append(element("div", "empty-state", "Chưa có thông báo."));
      return;
    }
    notifications.forEach((notification) => {
      const item = element("article", `notification-card${notification.isRead ? "" : " is-unread"}`);
      const text = element("p", "", notification.message);
      const date = element("time", "notification-time", notification.createdAt);
      item.append(element("span", "notification-dot"), text, date);
      if (!notification.isRead) {
        const button = element("button", "text-button", "Đánh dấu đã đọc");
        button.type = "button";
        button.addEventListener("click", async () => {
          try {
            await api(`/notifications/${notification.id}/read`, { method: "PUT" });
            await loadNotifications();
          } catch (error) {
            showMessage(error.message, "error");
          }
        });
        item.append(button);
      }
      list.append(item);
    });
  } catch (error) {
    list.replaceChildren(element("div", "empty-state", error.message));
  }
}

function initializeCustomer() {
  const today = new Date();
  const localToday = new Date(today.getTime() - today.getTimezoneOffset() * 60000)
    .toISOString().slice(0, 10);
  const start = document.querySelector('[name="startDate"]');
  const end = document.querySelector('[name="endDate"]');
  start.min = localToday;
  end.min = localToday;
  start.addEventListener("change", () => { end.min = start.value || localToday; });
  document.getElementById("rental-form").addEventListener("submit", submitRental);
  document.querySelectorAll("[data-vehicle-filter]").forEach((button) => {
    button.addEventListener("click", () => {
      document.querySelectorAll("[data-vehicle-filter]").forEach((item) => item.classList.remove("is-active"));
      button.classList.add("is-active");
      loadVehicles(button.dataset.vehicleFilter);
    });
  });
  loadVehicles();
  loadHistory();
  loadNotifications();
}

async function loadInventory(type = "all") {
  const container = document.getElementById("inventory-list");
  if (!container) return;
  try {
    const vehicles = await api(`/vehicles/inventory?type=${encodeURIComponent(type)}`);
    container.replaceChildren();
    if (!vehicles.length) {
      container.append(element("div", "empty-state", "Chưa có xe nào trong danh mục này."));
      return;
    }
    vehicles.forEach((vehicle) => {
      const card = element("article", "vehicle-card inventory-card");
      const tag = element("span", "vehicle-type", vehicle.type === "Car" ? "Ô TÔ" : "XE MÁY");
      const heading = element("h3", "", `${vehicle.brand} ${vehicle.model}`);
      const plate = element("p", "vehicle-plate", `Biển số · ${vehicle.licensePlate}`);
      const price = element("strong", "vehicle-price", money.format(vehicle.pricePerDay));
      price.append(element("span", "", " / ngày"));
      const availability = element("span", `status-pill ${vehicle.isAvailable ? "status-approved" : "status-pending"}`,
        vehicle.isAvailable ? "Có thể nhận lịch" : "Đang bảo dưỡng");
      const actions = element("div", "card-actions");
      const edit = element("button", "button button-secondary button-small", "Chỉnh sửa");
      edit.type = "button";
      edit.addEventListener("click", () => editVehicle(vehicle));
      const remove = element("button", "button button-danger button-small", "Xóa xe");
      remove.type = "button";
      remove.addEventListener("click", () => removeVehicle(vehicle));
      actions.append(edit, remove);
      card.append(tag, heading, plate, price, availability, actions);
      container.append(card);
    });
  } catch (error) {
    container.replaceChildren(element("div", "empty-state", error.message));
  }
}

function editVehicle(vehicle) {
  const form = document.getElementById("vehicle-form");
  form.elements.vehicleId.value = vehicle.id;
  form.elements.brand.value = vehicle.brand;
  form.elements.model.value = vehicle.model;
  form.elements.licensePlate.value = vehicle.licensePlate;
  form.elements.type.value = vehicle.type;
  form.elements.pricePerDay.value = vehicle.pricePerDay;
  form.elements.seatCount.value = vehicle.seatCount || "";
  form.elements.engineCapacity.value = vehicle.engineCapacity || "";
  document.getElementById("vehicle-form-title").textContent = `Chỉnh sửa ${vehicle.brand} ${vehicle.model}`;
  document.getElementById("vehicle-submit").textContent = "Lưu thay đổi";
  document.getElementById("cancel-vehicle-edit").hidden = false;
  form.scrollIntoView({ behavior: "smooth", block: "center" });
}

function resetVehicleForm() {
  const form = document.getElementById("vehicle-form");
  form.reset();
  form.elements.vehicleId.value = "";
  document.getElementById("vehicle-form-title").textContent = "Thêm xe mới";
  document.getElementById("vehicle-submit").textContent = "Thêm vào kho xe";
  document.getElementById("cancel-vehicle-edit").hidden = true;
}

async function saveVehicle(event) {
  event.preventDefault();
  const form = event.currentTarget;
  const vehicleId = form.elements.vehicleId.value;
  const payload = Object.fromEntries(new FormData(form));
  payload.pricePerDay = Number(payload.pricePerDay);
  payload.seatCount = payload.seatCount ? Number(payload.seatCount) : null;
  payload.engineCapacity = payload.engineCapacity ? Number(payload.engineCapacity) : null;
  const button = document.getElementById("vehicle-submit");
  button.disabled = true;
  try {
    await api(vehicleId ? `/vehicles/${vehicleId}` : "/vehicles", {
      method: vehicleId ? "PUT" : "POST",
      body: JSON.stringify(payload)
    });
    showMessage(vehicleId ? "Thông tin xe đã được cập nhật." : "Xe đã được thêm vào kho.");
    resetVehicleForm();
    await loadInventory();
  } catch (error) {
    showMessage(error.message, "error");
  } finally {
    button.disabled = false;
  }
}

async function removeVehicle(vehicle) {
  if (!window.confirm(`Xóa ${vehicle.brand} ${vehicle.model} (${vehicle.licensePlate}) khỏi kho xe?`)) return;
  try {
    await api(`/vehicles/${vehicle.id}`, { method: "DELETE" });
    showMessage("Xe đã được xóa khỏi danh sách.");
    await loadInventory();
  } catch (error) {
    showMessage(error.message, "error");
  }
}

async function loadPendingOrders() {
  const container = document.getElementById("pending-orders");
  try {
    const orders = await api("/rentals/pending");
    container.replaceChildren();
    if (!orders.length) {
      container.append(element("div", "empty-state", "Không có đơn thuê nào đang chờ duyệt."));
      return;
    }
    orders.forEach((order) => {
      const card = element("article", "order-card");
      const header = element("div", "order-card-heading");
      header.append(element("span", "order-number", `ĐƠN #${order.id}`));
      appendStatus(header, order.status);
      const title = element("h3", "", `${order.brand} ${order.model}`);
      const details = element("div", "order-details");
      [
        ["Khách hàng", order.customerUsername],
        ["Biển số", order.licensePlate],
        ["Thời gian", `${order.startDate} – ${order.endDate}`],
        ["CCCD", order.cccd],
        ["Bằng lái", order.driverLicense],
        ["Tổng tiền", money.format(order.totalAmount)],
        ["Tiền cọc", money.format(order.depositAmount)]
      ].forEach(([label, value]) => {
        const item = element("div", "order-detail");
        item.append(element("span", "", label), element("strong", "", String(value)));
        details.append(item);
      });
      const actions = element("div", "card-actions");
      const approve = element("button", "button button-primary button-small", "Duyệt đơn");
      approve.type = "button";
      approve.addEventListener("click", () => processOrder(order.id, "approve"));
      const reject = element("button", "button button-danger button-small", "Từ chối & hoàn cọc");
      reject.type = "button";
      reject.addEventListener("click", () => processOrder(order.id, "reject"));
      actions.append(approve, reject);
      card.append(header, title, details, actions);
      container.append(card);
    });
  } catch (error) {
    container.replaceChildren(element("div", "empty-state", error.message));
  }
}

async function processOrder(orderId, action) {
  const description = action === "reject"
    ? "Từ chối đơn này và ghi nhận hoàn tiền cọc cho khách hàng?"
    : "Duyệt đơn thuê này?";
  if (!window.confirm(description)) return;
  try {
    await api(`/rentals/${orderId}/${action}`, { method: "POST" });
    showMessage(action === "reject"
      ? "Đơn đã bị từ chối. Tiền cọc đã được ghi nhận hoàn."
      : "Đơn thuê đã được duyệt.");
    await Promise.all([loadPendingOrders(), loadCustomers()]);
  } catch (error) {
    showMessage(error.message, "error");
  }
}

async function loadCustomers() {
  const body = document.getElementById("customers-list");
  try {
    const customers = await api("/rentals/customers");
    body.replaceChildren();
    if (!customers.length) {
      const row = element("tr");
      const cell = element("td", "empty-cell", "Chưa có khách hàng hoàn tất đơn thuê.");
      cell.colSpan = 3;
      row.append(cell);
      body.append(row);
      return;
    }
    customers.forEach((customer) => {
      const row = element("tr");
      row.append(
        element("td", "table-strong", customer.username),
        element("td", "", String(customer.rentalCount)),
        element("td", "", customer.lastRental || "—")
      );
      body.append(row);
    });
  } catch (error) {
    const row = element("tr");
    const cell = element("td", "empty-cell", error.message);
    cell.colSpan = 3;
    row.replaceChildren(cell);
    body.replaceChildren(row);
  }
}

function initializeManager() {
  const form = document.getElementById("vehicle-form");
  form.addEventListener("submit", saveVehicle);
  document.getElementById("cancel-vehicle-edit").addEventListener("click", resetVehicleForm);
  document.querySelectorAll("[data-inventory-filter]").forEach((button) => {
    button.addEventListener("click", () => {
      document.querySelectorAll("[data-inventory-filter]").forEach((item) => item.classList.remove("is-active"));
      button.classList.add("is-active");
      loadInventory(button.dataset.inventoryFilter);
    });
  });
  loadInventory();
  loadPendingOrders();
  loadCustomers();
}

async function loadReport() {
  const report = document.getElementById("revenue-report");
  try {
    const data = await api("/admin/report");
    report.replaceChildren();
    [
      ["Doanh thu đơn đã duyệt", money.format(data.totalRevenue), "Tổng giá trị đơn Approved và Completed"],
      ["Tiền cọc ròng", money.format(data.netDeposits), "Đã trừ các khoản hoàn cọc"],
      ["Tổng đơn thuê", data.totalOrders, `${data.pendingOrders} đơn đang chờ duyệt`],
      ["Khách hàng", data.totalCustomers, "Tài khoản đã đăng ký"],
      ["Xe trong kho", data.totalVehicles, "Phương tiện đang được quản lý"]
    ].forEach(([label, value, note]) => {
      const card = element("article", "metric-card");
      card.append(element("span", "", label), element("strong", "", String(value)), element("small", "", note));
      report.append(card);
    });
    document.getElementById("revenue-note").textContent = data.revenueNote;
  } catch (error) {
    report.replaceChildren(element("div", "empty-state", error.message));
  }
}

async function loadManagers() {
  const list = document.getElementById("manager-list");
  try {
    const managers = await api("/admin/managers");
    list.replaceChildren();
    if (!managers.length) {
      list.append(element("div", "empty-state", "Chưa có tài khoản quản lý."));
      return;
    }
    managers.forEach((manager) => {
      const card = element("article", "manager-card");
      const info = element("div");
      info.append(element("span", "avatar avatar-small", "Q"), element("strong", "", manager.username));
      const remove = element("button", "button button-danger button-small", "Xóa tài khoản");
      remove.type = "button";
      remove.addEventListener("click", () => deleteManager(manager));
      card.append(info, remove);
      list.append(card);
    });
  } catch (error) {
    list.replaceChildren(element("div", "empty-state", error.message));
  }
}

async function addManager(event) {
  event.preventDefault();
  const form = event.currentTarget;
  const button = form.querySelector('button[type="submit"]');
  button.disabled = true;
  try {
    await api("/admin/managers", {
      method: "POST",
      body: JSON.stringify(Object.fromEntries(new FormData(form)))
    });
    showMessage("Đã tạo tài khoản quản lý.");
    form.reset();
    await loadManagers();
  } catch (error) {
    showMessage(error.message, "error");
  } finally {
    button.disabled = false;
  }
}

async function deleteManager(manager) {
  if (!window.confirm(`Xóa tài khoản quản lý "${manager.username}"?`)) return;
  try {
    await api(`/admin/managers/${manager.id}`, { method: "DELETE" });
    showMessage("Đã xóa tài khoản quản lý.");
    await loadManagers();
  } catch (error) {
    showMessage(error.message, "error");
  }
}

function initializeAdmin() {
  document.getElementById("manager-form").addEventListener("submit", addManager);
  document.getElementById("refresh-report").addEventListener("click", loadReport);
  loadReport();
  loadManagers();
  initializeManager();
}

async function initialize() {
  attachLogout();
  const page = document.body.dataset.page;
  if (page === "auth") return initializeAuthPage();
  const expectedRole = { customer: "Customer", manager: "Manager", admin: "Admin" }[page];
  if (!expectedRole) return;
  const user = await requirePageRole(expectedRole);
  if (!user) return;
  if (page === "customer") initializeCustomer();
  if (page === "manager") initializeManager();
  if (page === "admin") initializeAdmin();
}

document.addEventListener("DOMContentLoaded", () => {
  initialize().catch((error) => showMessage(error.message, "error"));
});
