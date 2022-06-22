#pragma once
#include "VMController.h"
#include "Database/VMSettings.h"
#include "GuacVNCClient.h"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <optional>
#include <vector>
#include <string>

class CollabVMServer;

class VMWareController : public VMController, public QMPCallback {
   public:
	enum ErrorCode {
        kPowercliError, // VMWare PowerCLI returned an error.
		kVNCFailed	// The VNC client failed to connect
	};

	/**
	 * Creates a new VM controller for VMWare.
	 */
	VMWareController(CollabVMServer& server, boost::asio::io_service& service, const std::shared_ptr<VMSettings>& settings);

	void ChangeSettings(const std::shared_ptr<VMSettings>& settings) override;

	~VMWareController() override;

	/**
	 * Run PowerCLI to start the VM
	 */
	void Start() override;

	/*
	 * Run PowerCLI to load the snapshot
	 */
	void RestoreVMSnapshot() override;

	void PowerOffVM() override;

	void ResetVM() override;

	void Stop(VMController::StopReason reason) override;

	void CleanUp() override;

	void OnGuacStarted() override;

	void OnGuacStopped() override;

	void OnGuacConnect() override;

	void OnGuacDisconnect(bool cleanup) override;

	const std::string& GetErrorMessage() const override;

	bool IsRunning() const override {
		return internal_state_ != InternalState::kInactive;
	}

	ControllerState GetState() const override;

	StopReason GetStopReason() const override {
		return stop_reason_;
	}

	inline void UpdateThumbnail() override {
		guac_client_.UpdateThumbnail();
	}

   protected:
	void OnAddUser(CollabVMUser& user) override;

	void OnRemoveUser(CollabVMUser& user) override;

   private:
	/**
	* Sets the VM name known to VMWare
	*/
	void SetVmwareName(const std::string& vmwarename);
    std::string VmwareName;
    /*
    * Sets the snapshot name to restore to on reset
    */
    void SetVmwareSnap(const std::string& vmwarename);
    std::string VmwareSnap;

	enum class InternalState {
		kInactive, // The controller hasn't been started
		kVNCConnecting, // The VNC client is connecting
		kConnected,		// The VNC client is connected (Which means the VM must be running)
		kStopping		// Waiting for VNC to stop.
	};

	GuacVNCClient guac_client_;

	/**
	 * The current state of the controller.
	 */
	InternalState internal_state_;

	/**
	 * The number of times the client has failed to connect to
	 * the VNC server. Once the count reaches the
	 * max number of attempts the client will stop trying and an
	 * error will occur.
	 */
	size_t retry_count_;

	ErrorCode error_code_;

#ifdef USE_SYSTEM_CLOCK
	typedef std::chrono::system_clock time_clock;
#else
	typedef std::chrono::steady_clock time_clock;
#endif

	/**
	 * This timer is used before attempting to connect to the VNC server
	 */
	boost::asio::steady_timer timer_;
};
